/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include "kio/bytecode.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace kio {

class JITEngine {
public:
    JITEngine();
    ~JITEngine();

    // Native loop function type: stack base, stack pointer reference, slots offset, globals map
    typedef void (*CompiledLoop)(Value* stack, int& sp, int slots, std::unordered_map<std::string, Value>& globals);

    CompiledLoop compileLoop(Chunk* chunk, uint8_t* startIp);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace kio
