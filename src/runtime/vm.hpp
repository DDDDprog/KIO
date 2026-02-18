/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include "axeon/bytecode.hpp"
#include "axeon/builtin_functions.hpp"
#include "axeon/jit_engine.hpp"
#include <vector>
#include <unordered_map>

namespace kio {

struct CallFrame {
    ObjFunction* function;
    uint8_t* ip;
    int slots; // Base pointer (offset into stack)
};

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class VM {
public:
    VM();
    ~VM();

    InterpretResult interpret(ObjFunction* function);
    void push(Value value);
    Value pop();

private:
    static constexpr int STACK_MAX = 8192;
    static constexpr int FRAMES_MAX = 128;

    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack_[STACK_MAX];
    int sp;
    std::unordered_map<std::string, Value> globals_;
    
    BuiltinFunctions builtins_;
    JITEngine jit_;
    std::unordered_map<uint8_t*, JITEngine::CompiledLoop> optimized_loops_;

    InterpretResult run();
    bool isTruthy(Value v);
    std::string valToString(Value v);
    
    bool callValue(Value callee, int argCount);
    bool call(ObjFunction* function, int argCount);
    bool invoke(const std::string& name, int argCount);
    bool bindMethod(ObjClass* klass, const std::string& name);
    
    std::unordered_map<uint8_t*, int> loop_hits_;
    static constexpr int HOT_THRESHOLD = 100;
};

} // namespace kio
