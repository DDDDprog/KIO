/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/vm.hpp"
#include <iostream>
#include <cstdio>
#include <chrono>
#include <cmath>

namespace kio {

VM::VM() {
    sp = 0;
    frameCount = 0;
}

VM::~VM() {}

bool VM::isTruthy(Value v) {
    if (isNil(v)) return false;
    if (isBool(v)) return v.as.boolean;
    if (isNumber(v)) return v.as.number != 0.0;
    return true;
}

InterpretResult VM::interpret(Chunk* chunk) {
    sp = 0;
    frameCount = 0;
    CallFrame* frame = &frames[frameCount++];
    frame->chunk = chunk;
    frame->ip = chunk->code.data();
    frame->slots = 0;
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
    }
    return "[Object]";
}

InterpretResult VM::run() {
    CallFrame* frame = &frames[frameCount - 1];
    uint8_t* ip = frame->ip;
    
    while (true) {
        OpCode inst = (OpCode)(*ip++);
        switch (inst) {
            case OpCode::CONSTANT: {
                uint8_t idx = *ip++;
                stack_[sp++] = frame->chunk->constants[idx];
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
                stack_[sp++] = Value(l.as.number + r.as.number);
                break;
            }
            case OpCode::SUBTRACT: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l.as.number - r.as.number);
                break;
            }
            case OpCode::MULTIPLY: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l.as.number * r.as.number);
                break;
            }
            case OpCode::DIVIDE: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l.as.number / r.as.number);
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
            case OpCode::GREATER: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l.as.number > r.as.number);
                break;
            }
            case OpCode::EQUAL: {
                Value r = stack_[--sp];
                Value l = stack_[--sp];
                stack_[sp++] = Value(l == r);
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
                if (!isTruthy(stack_[sp - 1])) ip += offset;
                break;
            }
            case OpCode::LOOP: {
                uint16_t offset = (uint16_t)((ip[0] << 8) | ip[1]);
                ip += 2;
                ip -= offset;
                break;
            }
            case OpCode::SYS_QUERY: {
                uint8_t idx = *ip++;
                std::string k = ((ObjString*)valueToObj(frame->chunk->constants[idx]))->chars;
                if (k == "time") {
                    auto now = std::chrono::steady_clock::now().time_since_epoch();
                    double ms = std::chrono::duration<double, std::milli>(now).count();
                    stack_[sp++] = Value(ms);
                }
                break;
            }
            case OpCode::HALT: return InterpretResult::OK;
            default: return InterpretResult::RUNTIME_ERROR;
        }
    }
}

} // namespace kio
