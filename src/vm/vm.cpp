/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/vm.hpp"
#include "axeon/bytecode.hpp"
#include "axeon/builtin_functions.hpp"
#include "axeon/asm_helpers.hpp"
#include "axeon/platform.hpp"
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

InterpretResult VM::interpret(ObjFunction* function) {
    push(objToValue(function));
    CallFrame* frame = &frames[frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code.data();
    frame->slots = 0;
    
    return run();
}

std::string VM::valToString(Value v) {
    if (isNil(v)) return "nil";
    if (isBool(v)) return (v.v == (0x7ff8000000000000 | 3)) ? "true" : "false";
    if (isNumber(v)) {
        char buf[64];
        double d = valueToDouble(v);
        if (d == (long long)d) snprintf(buf, sizeof(buf), "%lld", (long long)d);
        else snprintf(buf, sizeof(buf), "%.15g", d);
        return buf;
    }
    if (isObj(v)) {
        Obj* o = valueToObj(v);
        if (!o) return "nil";
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
        if (o->type == ObjType::OBJ_FUNCTION) return "<fn " + ((ObjFunction*)o)->name + ">";
        if (o->type == ObjType::OBJ_CLASS) return "<class " + ((ObjClass*)o)->name + ">";
        if (o->type == ObjType::OBJ_INSTANCE) return "<instance of " + ((ObjInstance*)o)->klass->name + ">";
    }
    return "[Object]";
}

bool VM::isTruthy(Value v) {
    if (isNil(v)) return false;
    if (isBool(v)) return v.v == (0x7ff8000000000000 | 3);
    if (isNumber(v)) return valueToDouble(v) != 0;
    return true;
}

InterpretResult VM::run() {
    CallFrame* frame = &frames[frameCount - 1];
    uint8_t* ip = frame->ip;
    Value* stack = stack_;
    int sp_local = sp;
    
    // Scratch variables
    std::string scratch_str;
    uint8_t scratch_byte;
    uint16_t scratch_u16;

#ifdef __GNUC__
    static void* dispatch_table[] = {
        &&code_CONSTANT, &&code_NIL, &&code_TRUE, &&code_FALSE, &&code_POP,
        &&code_GET_LOCAL, &&code_SET_LOCAL, &&code_GET_GLOBAL, &&code_DEFINE_GLOBAL, &&code_SET_GLOBAL,
        &&code_ADD, &&code_SUBTRACT, &&code_MULTIPLY, &&code_DIVIDE, &&code_MODULO,
        &&code_EQUAL, &&code_GREATER, &&code_GREATER_EQUAL, &&code_LESS, &&code_LESS_EQUAL,
        &&code_NOT, &&code_NEGATE, &&code_PRINT, &&code_JUMP, &&code_JUMP_IF_FALSE, &&code_LOOP,
        &&code_CALL, &&code_INVOKE, &&code_RETURN,
        &&code_CLASS, &&code_METHOD, &&code_GET_PROPERTY, &&code_SET_PROPERTY, &&code_INHERIT,
        &&code_ARRAY_NEW, &&code_ARRAY_GET, &&code_ARRAY_SET, &&code_SYS_QUERY,
        &&code_FLOOR, &&code_SQRT,
        &&code_FAST_LOOP, &&code_HALT
    };

    #define DISPATCH() { \
        /* printf("OP: %d, IP: %ld, SP: %d\n", (int)*ip, (long)(ip - frame->function->chunk.code.data()), sp_local); */ \
        goto *dispatch_table[*ip++]; \
    }
    DISPATCH();

code_CONSTANT:
    stack[sp_local++] = frame->function->chunk.constants[*ip++];
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
    stack[sp_local++] = stack[frame->slots + *ip++];
    DISPATCH();

code_SET_LOCAL:
    stack[frame->slots + *ip++] = stack[sp_local - 1];
    DISPATCH();

code_GET_GLOBAL: {
    scratch_byte = *ip++;
    scratch_str = ((ObjString*)valueToObj(frame->function->chunk.constants[scratch_byte]))->chars;
    if (globals_.find(scratch_str) == globals_.end()) {
        std::cerr << "Global '" << scratch_str << "' not found." << std::endl;
        stack[sp_local++] = NIL_VAL;
    } else {
        stack[sp_local++] = globals_[scratch_str];
    }
    DISPATCH();
}

code_DEFINE_GLOBAL: {
    scratch_byte = *ip++;
    scratch_str = ((ObjString*)valueToObj(frame->function->chunk.constants[scratch_byte]))->chars;
    globals_[scratch_str] = stack[--sp_local];
    // std::cout << "Defined global: " << scratch_str << std::endl;
    DISPATCH();
}

code_SET_GLOBAL: {
    scratch_byte = *ip++;
    scratch_str = ((ObjString*)valueToObj(frame->function->chunk.constants[scratch_byte]))->chars;
    globals_[scratch_str] = stack[sp_local - 1];
    DISPATCH();
}

code_ADD: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    if (isNumber(l) && isNumber(r)) {
        stack[sp_local++] = Value(valueToDouble(l) + valueToDouble(r));
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
    stack[sp_local++] = Value(valueToDouble(l) - valueToDouble(r));
    DISPATCH();
}

code_MULTIPLY: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(valueToDouble(l) * valueToDouble(r));
    DISPATCH();
}

code_DIVIDE: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(valueToDouble(l) / valueToDouble(r));
    DISPATCH();
}

code_MODULO: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(fmod(valueToDouble(l), valueToDouble(r)));
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
    stack[sp_local++] = Value(valueToDouble(l) > valueToDouble(r));
    DISPATCH();
}

code_GREATER_EQUAL: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(valueToDouble(l) >= valueToDouble(r));
    DISPATCH();
}

code_LESS: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(valueToDouble(l) < valueToDouble(r));
    DISPATCH();
}

code_LESS_EQUAL: {
    Value r = stack[--sp_local];
    Value l = stack[--sp_local];
    stack[sp_local++] = Value(valueToDouble(l) <= valueToDouble(r));
    DISPATCH();
}

code_NOT:
    stack[sp_local - 1] = Value(!isTruthy(stack[sp_local - 1]));
    DISPATCH();

code_NEGATE:
    stack[sp_local - 1] = Value(-valueToDouble(stack[sp_local - 1]));
    DISPATCH();

code_PRINT: {
    std::cout << valToString(stack[--sp_local]) << std::endl;
    DISPATCH();
}

code_JUMP: {
    scratch_u16 = (uint16_t)((ip[0] << 8) | ip[1]);
    ip += 2 + scratch_u16;
    DISPATCH();
}

code_JUMP_IF_FALSE: {
    scratch_u16 = (uint16_t)((ip[0] << 8) | ip[1]);
    ip += 2;
    if (!isTruthy(stack[--sp_local])) ip += scratch_u16;
    DISPATCH();
}

code_LOOP: {
    scratch_u16 = (uint16_t)((ip[0] << 8) | ip[1]);
    ip += 2;
    uint8_t* target_ip = ip - scratch_u16;
    
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
        JITEngine::CompiledLoop compiled = jit_.compileLoop(&frame->function->chunk, target_ip);
        
        if (compiled) {
            optimized_loops_[target_ip] = compiled;
            compiled(stack, sp, frame->slots, globals_);
        } else {
            std::cerr << "[JIT] Failed to compile loop at offset " << (int)(target_ip - frame->function->chunk.code.data()) << std::endl;
            optimized_loops_[target_ip] = nullptr; 
        }
        sp_local = sp;
    }
    
    ip = target_ip;
    DISPATCH();
}

code_CALL: {
    scratch_byte = *ip++;
    sp = sp_local;
    if (!callValue(stack[sp - scratch_byte - 1], scratch_byte)) {
        return InterpretResult::RUNTIME_ERROR;
    }
    frame = &frames[frameCount - 1];
    ip = frame->ip;
    sp_local = sp;
    DISPATCH();
}

code_INVOKE: {
    scratch_byte = *ip++; // constant index for name
    int argCount = *ip++;
    scratch_str = ((ObjString*)valueToObj(frame->function->chunk.constants[scratch_byte]))->chars;
    sp = sp_local;
    if (!invoke(scratch_str, argCount)) return InterpretResult::RUNTIME_ERROR;
    frame = &frames[frameCount - 1];
    ip = frame->ip;
    sp_local = sp;
    DISPATCH();
}

code_RETURN: {
    Value result = stack[--sp_local];
    frameCount--;
    if (frameCount == 0) {
        sp = sp_local;
        return InterpretResult::OK;
    }
    sp = frame->slots;
    stack[sp++] = result;
    frame = &frames[frameCount - 1];
    ip = frame->ip;
    sp_local = sp;
    DISPATCH();
}

code_CLASS: {
    scratch_byte = *ip++;
    scratch_str = ((ObjString*)valueToObj(frame->function->chunk.constants[scratch_byte]))->chars;
    stack[sp_local++] = objToValue(new ObjClass(scratch_str));
    DISPATCH();
}

code_METHOD: {
    scratch_byte = *ip++;
    scratch_str = ((ObjString*)valueToObj(frame->function->chunk.constants[scratch_byte]))->chars;
    Value method = stack[--sp_local];
    ObjClass* klass = (ObjClass*)valueToObj(stack[sp_local - 1]);
    klass->methods[scratch_str] = method;
    DISPATCH();
}

code_GET_PROPERTY: {
    if (!isObj(stack[sp_local - 1]) || valueToObj(stack[sp_local - 1])->type != ObjType::OBJ_INSTANCE) {
        std::cerr << "Only instances have properties." << std::endl;
        return InterpretResult::RUNTIME_ERROR;
    }
    ObjInstance* instance = (ObjInstance*)valueToObj(stack[--sp_local]);
    scratch_byte = *ip++;
    scratch_str = ((ObjString*)valueToObj(frame->function->chunk.constants[scratch_byte]))->chars;
    
    auto it = instance->fields.find(scratch_str);
    if (it != instance->fields.end()) {
        stack[sp_local++] = it->second;
    } else {
        // Look in methods
        auto mit = instance->klass->methods.find(scratch_str);
        if (mit != instance->klass->methods.end()) {
            stack[sp_local++] = mit->second;
        } else {
            stack[sp_local++] = NIL_VAL;
        }
    }
    DISPATCH();
}

code_SET_PROPERTY: {
    if (!isObj(stack[sp_local - 2]) || valueToObj(stack[sp_local - 2])->type != ObjType::OBJ_INSTANCE) {
        std::cerr << "Only instances have properties." << std::endl;
        return InterpretResult::RUNTIME_ERROR;
    }
    ObjInstance* instance = (ObjInstance*)valueToObj(stack[sp_local - 2]);
    scratch_byte = *ip++;
    scratch_str = ((ObjString*)valueToObj(frame->function->chunk.constants[scratch_byte]))->chars;
    instance->fields[scratch_str] = stack[sp_local - 1];
    Value value = stack[--sp_local];
    sp_local--; // obj
    stack[sp_local++] = value;
    DISPATCH();
}

code_INHERIT: {
    // Basic inheritance not yet fully implemented in parser, but opcode ready
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
    scratch_byte = *ip++;
    scratch_str = ((ObjString*)valueToObj(frame->function->chunk.constants[scratch_byte]))->chars;
    if (scratch_str == "time") {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        double ms = std::chrono::duration<double, std::milli>(now).count();
        stack[sp_local++] = Value(ms);
    } else if (scratch_str == "os_name") {
#ifdef _WIN32
        stack[sp_local++] = objToValue(new ObjString("Windows"));
#elif __APPLE__
        stack[sp_local++] = objToValue(new ObjString("macOS"));
#else
        stack[sp_local++] = objToValue(new ObjString("Linux"));
#endif
    } else if (scratch_str == "arch") {
        stack[sp_local++] = objToValue(new ObjString("x64"));
    } else if (scratch_str == "kio_version") {
        stack[sp_local++] = objToValue(new ObjString("2.1.0"));
    } else if (scratch_str == "cpu_model") {
        stack[sp_local++] = objToValue(new ObjString(PlatformInfo::get_cpu_model()));
    } else if (scratch_str == "mem_total_kb") {
        stack[sp_local++] = Value((double)PlatformInfo::get_total_memory());
    } else if (scratch_str == "disk_root_kb") {
        stack[sp_local++] = Value((double)PlatformInfo::get_root_disk_space());
    } else {
        stack[sp_local++] = Value();
    }
    DISPATCH();
}

code_FLOOR:
    stack[sp_local - 1] = Value(std::floor(valueToDouble(stack[sp_local - 1])));
    DISPATCH();

code_SQRT:
    stack[sp_local - 1] = Value(std::sqrt(valueToDouble(stack[sp_local - 1])));
    DISPATCH();

code_FAST_LOOP: {
    // Basic fast loop implementation if needed
    DISPATCH();
}

code_HALT:
    sp = sp_local;
    return InterpretResult::OK;

#else
    // Legacy switch-based dispatcher removed for brevity
    return InterpretResult::RUNTIME_ERROR;
#endif
}

bool VM::callValue(Value callee, int argCount) {
    if (isObj(callee)) {
        Obj* o = valueToObj(callee);
        if (!o) {
            std::cerr << "Callee is a null object." << std::endl;
            return false;
        }
        switch (o->type) {
            case ObjType::OBJ_FUNCTION:
                return call((ObjFunction*)o, argCount);
            case ObjType::OBJ_CLASS: {
                ObjClass* klass = (ObjClass*)o;
                stack_[sp - argCount - 1] = objToValue(new ObjInstance(klass));
                return true;
            }
            case ObjType::OBJ_STRING: {
                std::string name = ((ObjString*)o)->chars;
                if (builtins_.hasFunction(name)) {
                    std::vector<Value> args;
                    for (int i = 0; i < argCount; ++i) {
                        args.push_back(stack_[sp - argCount + i]);
                    }
                    sp -= argCount + 1;
                    push(builtins_.callFunction(name, args));
                    return true;
                }
                std::cerr << "Native function '" << name << "' not found." << std::endl;
                break;
            }
            default: 
                std::cerr << "Object type " << (int)o->type << " is not callable." << std::endl;
                break;
        }
    } else {
        std::cerr << "Callee is not an object. Raw bits: " << std::hex << callee.v << std::dec << std::endl;
        if (isNil(callee)) std::cerr << "Callee is nil." << std::endl;
        if (isBool(callee)) std::cerr << "Callee is bool." << std::endl;
        if (isNumber(callee)) std::cerr << "Callee is number: " << valueToDouble(callee) << std::endl;
    }
    return false;
}

bool VM::call(ObjFunction* function, int argCount) {
    if (argCount != function->arity) {
        std::cerr << "Expected " << function->arity << " arguments but got " << argCount << "." << std::endl;
        return false;
    }
    if (frameCount == FRAMES_MAX) {
        std::cerr << "Stack overflow." << std::endl;
        return false;
    }
    CallFrame* frame = &frames[frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code.data();
    frame->slots = sp - argCount - 1;
    return true;
}

bool VM::invoke(const std::string& name, int argCount) {
    Value receiver = stack_[sp - argCount - 1];
    if (!isObj(receiver) || valueToObj(receiver)->type != ObjType::OBJ_INSTANCE) {
        std::cerr << "Only instances have methods." << std::endl;
        return false;
    }
    ObjInstance* instance = (ObjInstance*)valueToObj(receiver);
    auto it = instance->klass->methods.find(name);
    if (it == instance->klass->methods.end()) {
        std::cerr << "Undefined method '" << name << "'." << std::endl;
        return false;
    }
    return call((ObjFunction*)valueToObj(it->second), argCount);
}

void VM::push(Value value) {
    stack_[sp++] = value;
}

Value VM::pop() {
    return stack_[--sp];
}

} // namespace kio
