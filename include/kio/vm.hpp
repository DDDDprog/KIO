/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include "kio/bytecode.hpp"
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

    InterpretResult run();
    bool isTruthy(Value v);
};

} // namespace kio
