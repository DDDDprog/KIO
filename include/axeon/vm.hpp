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
    uint8_t* ip;
    Chunk* chunk;
    int slots; // Offset into stack
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

    InterpretResult interpret(Chunk* chunk);
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

    std::unordered_map<uint8_t*, int> loop_hits_;
    static constexpr int HOT_THRESHOLD = 100;
};

} // namespace kio
