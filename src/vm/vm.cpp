/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/vm.hpp"
#include "kio/bytecode.hpp"
#include "kio/builtin_functions.hpp"
#include "kio/asm_helpers.hpp"
#include <iostream>
#include <cmath>
#include <chrono>
#include <map>

namespace kio {

VM::VM() {
    sp = 0;
    frameCount = 0;
    for (const auto& pair : builtins_.getFunctionNames()) {
        globals_[pair] = objToValue(new ObjString(pair));
    }
}

VM::~VM() {}

InterpretResult VM::interpret(Chunk* chunk) {
    frameCount = 1;
    CallFrame* frame = &frames[0];
    frame->chunk = chunk;
    frame->ip = chunk->code.data();
    frame->slots = 0;
    sp = 0; // Ensure stack is clean
    return run();
}

static std::string valToString(Value v) {
    if (isNil(v)) return "nil";
    if (isBool(v)) return v.as.boolean ? "true" : "false";
    if (isNumber(v)) {
        char buf[64];
        double d = v.as.number;
        if (d == (long long)d) snprintf(buf, sizeof(buf), "%lld", (long long)d);
        else snprintf(buf, sizeof(buf), "%.15g", d);
        return buf;
    }
    if (isObj(v)) {
        Obj* o = v.as.obj;
        if (o->type == ObjType::OBJ_STRING) return ((ObjString*)o)->chars;
        if (o->type == ObjType::OBJ_ARRAY) {
            std::string res = "[";
            ObjArray* arr = (ObjArray*)o;
            for (size_t i = 0; i < arr->elements.size(); ++i) {
                res += valToString(arr->elements[i]);
                if (i < arr->elements.size() - 1) res += ", ";
            }
            res += "]";
            return res;
        }
    }
    return "[Object]";
}

bool VM::isTruthy(Value v) {
    if (isNil(v)) return false;
    if (isBool(v)) return v.as.boolean;
    if (isNumber(v)) return v.as.number != 0;
    return true;
}

InterpretResult VM::run() {
    CallFrame* frame = &frames[frameCount - 1];
    uint8_t* ip = frame->ip;
    
    while (true) {
        if (sp >= STACK_MAX) {
            std::cerr << "Stack overflow!" << std::endl;
            return InterpretResult::RUNTIME_ERROR;
        }
        
        OpCode inst = (OpCode)(*ip++);
        
        switch (inst) {
            case OpCode::CONSTANT: {
                stack_[sp++] = frame->chunk->constants[*ip++];
                break;
            }
            case OpCode::NIL:   stack_[sp++] = Value(); break;
            case OpCode::TRUE:  stack_[sp++] = Value(true); break;
            case OpCode::FALSE: stack_[sp++] = Value(false); break;
            case OpCode::POP:   sp--; break;
            case OpCode::GET_LOCAL: {
                uint8_t slot = *ip++;
                stack_[sp++] = stack_[slot];
                break;
            }
            case OpCode::SET_LOCAL: {
                uint8_t slot = *ip++;
                stack_[slot] = stack_[sp - 1];
                break;
            }
            case OpCode::GET_GLOBAL: {
                uint8_t idx = *ip++;
                std::string n = ((ObjString*)valueToObj(frame->chunk->constants[idx]))->chars;
                stack_[sp++] = globals_[n];
                break;
            }
            case OpCode::DEFINE_GLOBAL: {
                uint8_t idx = *ip++;
                std::string n = ((ObjString*)valueToObj(frame->chunk->constants[idx]))->chars;
                globals_[n] = stack_[--sp];
                break;
            }
            case OpCode::SET_GLOBAL: {
                uint8_t idx = *ip++;
                std::string n = ((ObjString*)valueToObj(frame->chunk->constants[idx]))->chars;
                globals_[n] = stack_[sp - 1];
                break;
            }
            case OpCode::ADD: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                if ((isObj(l) && valueToObj(l)->type == ObjType::OBJ_STRING) ||
                    (isObj(r) && valueToObj(r)->type == ObjType::OBJ_STRING)) {
                    std::string s1 = valToString(l);
                    std::string s2 = valToString(r);
                    stack_[sp++] = objToValue(new ObjString(s1 + s2));
                } else {
                    stack_[sp++] = Value(asm_add_double(l.as.number, r.as.number));
                }
                break;
            }
            case OpCode::SUBTRACT: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(asm_sub_double(l.as.number, r.as.number));
                break;
            }
            case OpCode::MULTIPLY: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(asm_mul_double(l.as.number, r.as.number));
                break;
            }
            case OpCode::DIVIDE: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(asm_div_double(l.as.number, r.as.number));
                break;
            }
            case OpCode::MODULO: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(fmod(l.as.number, r.as.number));
                break;
            }
            case OpCode::LESS: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l.as.number < r.as.number);
                break;
            }
            case OpCode::LESS_EQUAL: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l.as.number <= r.as.number);
                break;
            }
            case OpCode::GREATER: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l.as.number > r.as.number);
                break;
            }
            case OpCode::GREATER_EQUAL: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l.as.number >= r.as.number);
                break;
            }
            case OpCode::EQUAL: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l == r);
                break;
            }
            case OpCode::NOT: {
                stack_[sp - 1] = Value(!isTruthy(stack_[sp - 1]));
                break;
            }
            case OpCode::NEGATE: {
                if (!isNumber(stack_[sp - 1])) return InterpretResult::RUNTIME_ERROR;
                stack_[sp - 1] = Value(-stack_[sp - 1].as.number);
                break;
            }
            case OpCode::PRINT: {
                std::cout << valToString(stack_[--sp]) << std::endl;
                break;
            }
            case OpCode::JUMP: {
                uint16_t offset = (uint16_t)((ip[0] << 8) | ip[1]);
                ip += 2 + offset;
                break;
            }
            case OpCode::JUMP_IF_FALSE: {
                uint16_t offset = (uint16_t)((ip[0] << 8) | ip[1]);
                ip += 2;
                if (!isTruthy(stack_[--sp])) ip += offset;
                break;
            }
            case OpCode::LOOP: {
                uint16_t offset = (uint16_t)((ip[0] << 8) | ip[1]);
                ip += 2;
                ip -= offset;
                
                // Hot-loop detection and JIT execution
                if (optimized_loops_.count(ip)) {
                    optimized_loops_[ip](stack_, sp, frame->slots, globals_);
                } else if (++loop_hits_[ip] == HOT_THRESHOLD) {
                    std::cout << "[JIT] Hot loop detected. Compiling to native LLVM..." << std::endl;
                    auto nativeLoop = jit_.compileLoop(frame->chunk, ip);
                    if (nativeLoop) {
                        optimized_loops_[ip] = nativeLoop;
                        nativeLoop(stack_, sp, frame->slots, globals_);
                    }
                }
                break;
            }
            case OpCode::SYS_QUERY: {
                uint8_t idx = *ip++;
                std::string k = ((ObjString*)valueToObj(frame->chunk->constants[idx]))->chars;
                if (k == "time") {
                    auto now = std::chrono::steady_clock::now().time_since_epoch();
                    double ms = std::chrono::duration<double, std::milli>(now).count();
                    stack_[sp++] = Value(ms);
                } else if (k == "os_name") {
#ifdef _WIN32
                    stack_[sp++] = objToValue(new ObjString("Windows"));
#elif __APPLE__
                    stack_[sp++] = objToValue(new ObjString("macOS"));
#else
                    stack_[sp++] = objToValue(new ObjString("Linux"));
#endif
                } else if (k == "os_version") {
                    stack_[sp++] = objToValue(new ObjString("1.0.0"));
                } else if (k == "arch") {
#if defined(__x86_64__) || defined(_M_X64)
                    stack_[sp++] = objToValue(new ObjString("x64"));
#elif defined(__i386__) || defined(_M_IX86)
                    stack_[sp++] = objToValue(new ObjString("x86"));
#elif defined(__arm64__) || defined(__aarch64__)
                    stack_[sp++] = objToValue(new ObjString("ARM64"));
#else
                    stack_[sp++] = objToValue(new ObjString("Unknown"));
#endif
                } else if (k == "kio_version") {
                    stack_[sp++] = objToValue(new ObjString("2.1.0"));
                } else {
                    stack_[sp++] = Value();
                }
                break;
            }
            case OpCode::CALL: {
                uint8_t argCount = *ip++;
                Value callee = stack_[--sp];
                if (isObj(callee) && valueToObj(callee)->type == ObjType::OBJ_STRING) {
                    std::string funcName = ((ObjString*)valueToObj(callee))->chars;
                    if (builtins_.hasFunction(funcName)) {
                        std::vector<Value> args;
                        for (int i = 0; i < argCount; ++i) {
                            args.push_back(stack_[sp - argCount + i]);
                        }
                        sp -= argCount;
                        stack_[sp++] = builtins_.callFunction(funcName, args);
                    } else {
                        std::cerr << "Undefined function: " << funcName << std::endl;
                        return InterpretResult::RUNTIME_ERROR;
                    }
                } else {
                    std::cerr << "Can only call functions (strings)." << std::endl;
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::ARRAY_NEW: {
                uint8_t elementCount = *ip++;
                ObjArray* array = new ObjArray();
                array->elements.resize(elementCount);
                for (int i = elementCount - 1; i >= 0; --i) {
                    array->elements[i] = stack_[--sp];
                }
                stack_[sp++] = objToValue(array);
                break;
            }
            case OpCode::ARRAY_GET: {
                Value index = stack_[--sp];
                Value arrayVal = stack_[--sp];
                if (!isObj(arrayVal) || valueToObj(arrayVal)->type != ObjType::OBJ_ARRAY) {
                    std::cerr << "ARRAY_GET: Expected array, got " << valToString(arrayVal) << " (Type: " << (int)arrayVal.type << ")" << std::endl;
                    return InterpretResult::RUNTIME_ERROR;
                }
                ObjArray* array = (ObjArray*)valueToObj(arrayVal);
                int idx = (int)index.toNumber();
                if (idx < 0 || idx >= (int)array->elements.size()) return InterpretResult::RUNTIME_ERROR;
                stack_[sp++] = array->elements[idx];
                break;
            }
            case OpCode::ARRAY_SET: {
                Value value = stack_[--sp];
                Value index = stack_[--sp];
                Value arrayVal = stack_[--sp];
                if (!isObj(arrayVal) || valueToObj(arrayVal)->type != ObjType::OBJ_ARRAY) return InterpretResult::RUNTIME_ERROR;
                ObjArray* array = (ObjArray*)valueToObj(arrayVal);
                int idx = (int)index.toNumber();
                if (idx < 0 || idx >= (int)array->elements.size()) return InterpretResult::RUNTIME_ERROR;
                array->elements[idx] = value;
                stack_[sp++] = value;
                break;
            }
            case OpCode::HALT: return InterpretResult::OK;
            default: return InterpretResult::RUNTIME_ERROR;
        }
    }
}

} // namespace kio
