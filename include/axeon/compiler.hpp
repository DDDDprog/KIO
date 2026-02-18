/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include "axeon/ast.hpp"
#include "axeon/bytecode.hpp"

namespace kio {

class Compiler {
public:
    enum class FunctionType { TYPE_FUNCTION, TYPE_SCRIPT };

    Compiler(Compiler* parent = nullptr, FunctionType type = FunctionType::TYPE_SCRIPT);
    ObjFunction* compile(const std::vector<StmtPtr>& statements);

private:
    struct Local {
        std::string name;
        int depth;
    };

    Compiler* parent_;
    ObjFunction* function_;
    FunctionType type_;

    std::vector<Local> locals_;
    int scopeDepth {0};

    void compileStmt(const StmtPtr& stmt);
    void compileExpr(const ExprPtr& expr);
    
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t b1, uint8_t b2);
    void emitBytes(uint8_t b1, uint8_t b2, uint8_t b3);
    void emitBytes(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);
    int addConstant(Value value);
    void emitConstant(Value value);
    int emitJump(OpCode instruction);
    void patchJump(int offset);
    void emitLoop(int loopStart);

    void addLocal(const std::string& name);
    int resolveLocal(const std::string& name);
    
    Chunk* currentChunk() { return &function_->chunk; }
};

} // namespace kio
