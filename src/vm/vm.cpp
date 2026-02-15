/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/vm.hpp"
#include "axeon/bytecode.hpp"
#include "axeon/builtin_functions.hpp"
#include "axeon/asm_helpers.hpp"
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
    register uint8_t* ip = frame->ip;
    register Value* stack = stack_;
    register int sp_local = sp;

#ifdef __GNUC__
    static void* dispatch_table[] = {
        &&code_CONSTANT, &&code_NIL, &&code_TRUE, &&code_FALSE, &&code_POP,
        &&code_GET_LOCAL, &&code_SET_LOCAL, &&code_GET_GLOBAL, &&code_DEFINE_GLOBAL, &&code_SET_GLOBAL,
        &&code_ADD, &&code_SUBTRACT, &&code_MULTIPLY, &&code_DIVIDE, &&code_MODULO,
        &&code_EQUAL, &&code_GREATER, &&code_GREATER_EQUAL, &&code_LESS, &&code_LESS_EQUAL,
        &&code_NOT, &&code_NEGATE, &&code_PRINT, &&code_JUMP, &&code_JUMP_IF_FALSE, &&code_LOOP,
        &&code_CALL, &&code_ARRAY_NEW, &&code_ARRAY_GET, &&code_ARRAY_SET, &&code_SYS_QUERY,
        &&code_FAST_LOOP, &&code_HALT
    };

    #define DISPATCH() goto *dispatch_table[*ip++]
    DISPATCH();

code_CONSTANT:
    stack[sp_local++] = frame->chunk->constants[*ip++];
    DISPATCH();

code_NIL:
    stack[sp_local++] = Value();
    DISPATCH();

code_TRUE:
    stack[sp_local++] = Value(true);
    DISPATCH();

code_FALSE:
    stack[sp_local++] = Value(false);
    DISPATCH();

code_POP:
    sp_local--;
    DISPATCH();

code_GET_LOCAL:
    stack[sp_local++] = stack[*ip++];
    DISPATCH();

code_SET_LOCAL:
    stack[*ip++] = stack[sp_local - 1];
    DISPATCH();

code_GET_GLOBAL: {
    uint8_t idx = *ip++;
    std::string n = ((ObjString*)valueToObj(frame->chunk->constants[idx]))->chars;
    stack[sp_local++] = globals_[n];
    DISPATCH();
}

code_DEFINE_GLOBAL: {
    uint8_t idx = *ip++;
    std::string n = ((ObjString*)valueToObj(frame->chunk->constants[idx]))->chars;
    globals_[n] = stack[--sp_local];
    DISPATCH();
}

code_SET_GLOBAL: {
    uint8_t idx = *ip++;
    std::string n = ((ObjString*)valueToObj(frame->chunk->constants[idx]))->chars;
    globals_[n] = stack[sp_local - 1];
    DISPATCH();
}

code_ADD: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    if (isNumber(l) && isNumber(r)) {
        stack[sp_local++] = Value(l.as.number + r.as.number);
    } else if (isObj(l) || isObj(r)) {
        std::string s1 = valToString(l);
        std::string s2 = valToString(r);
        stack[sp_local++] = objToValue(new ObjString(s1 + s2));
    } else {
        stack[sp_local++] = NIL_VAL;
    }
    DISPATCH();
}

code_SUBTRACT: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(l.as.number - r.as.number);
    DISPATCH();
}

code_MULTIPLY: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(l.as.number * r.as.number);
    DISPATCH();
}

code_DIVIDE: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(l.as.number / r.as.number);
    DISPATCH();
}

code_MODULO: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(fmod(l.as.number, r.as.number));
    DISPATCH();
}

code_EQUAL: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(l == r);
    DISPATCH();
}

code_GREATER: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(l.as.number > r.as.number);
    DISPATCH();
}

code_GREATER_EQUAL: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(l.as.number >= r.as.number);
    DISPATCH();
}

code_LESS: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(l.as.number < r.as.number);
    DISPATCH();
}

code_LESS_EQUAL: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(l.as.number <= r.as.number);
    DISPATCH();
}

code_NOT:
    stack[sp_local - 1] = Value(!isTruthy(stack[sp_local - 1]));
    DISPATCH();

code_NEGATE:
    stack[sp_local - 1] = Value(-stack[sp_local - 1].as.number);
    DISPATCH();

code_PRINT:
    std::cout << valToString(stack[--sp_local]) << std::endl;
    DISPATCH();

code_JUMP: {
    uint16_t offset = (uint16_t)((ip[0] << 8) | ip[1]);
    ip += 2 + offset;
    DISPATCH();
}

code_JUMP_IF_FALSE: {
    uint16_t offset = (uint16_t)((ip[0] << 8) | ip[1]);
    ip += 2;
    if (!isTruthy(stack[--sp_local])) ip += offset;
    DISPATCH();
}

code_LOOP: {
    uint16_t offset = (uint16_t)((ip[0] << 8) | ip[1]);
    ip += 2;
    uint8_t* target_ip = ip - offset;
    
    auto it = optimized_loops_.find(target_ip);
    if (it != optimized_loops_.end()) {
        if (it->second) {
            sp = sp_local;
            it->second(stack, sp, frame->slots, globals_);
            sp_local = sp;
            ip = target_ip;
            DISPATCH();
        }
    } else if (++loop_hits_[target_ip] >= HOT_THRESHOLD) {
        sp = sp_local;
        JITEngine::CompiledLoop compiled = jit_.compileLoop(frame->chunk, target_ip);
        
        if (compiled) {
            optimized_loops_[target_ip] = compiled;
            compiled(stack, sp, frame->slots, globals_);
        } else {
            optimized_loops_[target_ip] = nullptr; 
        }
        sp_local = sp;
    }
    
    ip = target_ip;
    DISPATCH();
}

code_CALL: {
    uint8_t argCount = *ip++;
    Value callee = stack[--sp_local];
    if (isObj(callee) && valueToObj(callee)->type == ObjType::OBJ_STRING) {
        std::string funcName = ((ObjString*)valueToObj(callee))->chars;
        if (builtins_.hasFunction(funcName)) {
            std::vector<Value> args;
            for (int i = 0; i < argCount; ++i) {
                args.push_back(stack[sp_local - argCount + i]);
            }
            sp_local -= argCount;
            stack[sp_local++] = builtins_.callFunction(funcName, args);
        } else {
            std::cerr << "Undefined function: " << funcName << std::endl;
            return InterpretResult::RUNTIME_ERROR;
        }
    } else {
        std::cerr << "Can only call functions (strings)." << std::endl;
        return InterpretResult::RUNTIME_ERROR;
    }
    DISPATCH();
}

code_ARRAY_NEW: {
    uint8_t elementCount = *ip++;
    ObjArray* array = new ObjArray();
    array->elements.resize(elementCount);
    for (int i = elementCount - 1; i >= 0; --i) {
        array->elements[i] = stack[--sp_local];
    }
    stack[sp_local++] = objToValue(array);
    DISPATCH();
}

code_ARRAY_GET: {
    Value index = stack[--sp_local];
    Value arrayVal = stack[--sp_local];
    ObjArray* array = (ObjArray*)valueToObj(arrayVal);
    int idx = (int)index.toNumber();
    stack[sp_local++] = array->elements[idx];
    DISPATCH();
}

code_ARRAY_SET: {
    Value value = stack[--sp_local];
    Value index = stack[--sp_local];
    Value arrayVal = stack[--sp_local];
    ObjArray* array = (ObjArray*)valueToObj(arrayVal);
    int idx = (int)index.toNumber();
    array->elements[idx] = value;
    stack[sp_local++] = value;
    DISPATCH();
}

code_SYS_QUERY: {
    uint8_t idx = *ip++;
    std::string k = ((ObjString*)valueToObj(frame->chunk->constants[idx]))->chars;
    if (k == "time") {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        double ms = std::chrono::duration<double, std::milli>(now).count();
        stack[sp_local++] = Value(ms);
    } else if (k == "os_name") {
#ifdef _WIN32
        stack[sp_local++] = objToValue(new ObjString("Windows"));
#elif __APPLE__
        stack[sp_local++] = objToValue(new ObjString("macOS"));
#else
        stack[sp_local++] = objToValue(new ObjString("Linux"));
#endif
    } else if (k == "arch") {
        stack[sp_local++] = objToValue(new ObjString("x64"));
    } else if (k == "kio_version") {
        stack[sp_local++] = objToValue(new ObjString("2.1.0"));
    } else {
        stack[sp_local++] = Value();
    }
    DISPATCH();
}

code_FAST_LOOP: {
    // Basic fast loop implementation if needed
    DISPATCH();
}

code_HALT:
    sp = sp_local;
    return InterpretResult::OK;

#else
    while (true) {
        OpCode inst = (OpCode)(*ip++);
        switch (inst) {
            case OpCode::CONSTANT: stack[sp_local++] = frame->chunk->constants[*ip++]; break;
            case OpCode::ADD: {
                Value r = stack[--sp_local];
                Value l = stack[--sp_local];
                stack[sp_local++] = Value(l.as.number + r.as.number);
                break;
            }
            case OpCode::SUBTRACT: {
                Value r = stack[--sp_local];
                Value l = stack[--sp_local];
                stack[sp_local++] = Value(l.as.number - r.as.number);
                break;
            }
            case OpCode::MULTIPLY: {
                Value r = stack[--sp_local];
                Value l = stack[--sp_local];
                stack[sp_local++] = Value(l.as.number * r.as.number);
                break;
            }
            case OpCode::DIVIDE: {
                Value r = stack[--sp_local];
                Value l = stack[--sp_local];
                stack[sp_local++] = Value(l.as.number / r.as.number);
                break;
            }
            case OpCode::LESS: {
                Value r = stack[--sp_local];
                Value l = stack[--sp_local];
                stack[sp_local++] = Value(l.as.number < r.as.number);
                break;
            }
            case OpCode::JUMP_IF_FALSE: {
                uint16_t offset = (uint16_t)((ip[0] << 8) | ip[1]);
                ip += 2;
                if (!isTruthy(stack[--sp_local])) ip += offset;
                break;
            }
            case OpCode::LOOP: {
                uint16_t offset = (uint16_t)((ip[0] << 8) | ip[1]);
                ip += 2;
                ip -= offset;
                break;
            }
            case OpCode::SET_LOCAL: {
                uint8_t slot = *ip++;
                stack[slot] = stack[sp_local - 1];
                break;
            }
            case OpCode::GET_LOCAL: {
                uint8_t slot = *ip++;
                stack[sp_local++] = stack[slot];
                break;
            }
            case OpCode::HALT:
                sp = sp_local;
                return InterpretResult::OK;
            default: break;
        }
    }
#endif
}

} // namespace kio
