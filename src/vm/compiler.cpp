/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/compiler.hpp"
#include <iostream>

namespace kio {

Compiler::Compiler() : chunk_(nullptr) {}

Chunk* Compiler::compile(const std::vector<StmtPtr>& statements) {
    chunk_ = new Chunk();
    for (const auto& stmt : statements) { compileStmt(stmt); }
    emitByte(static_cast<uint8_t>(OpCode::HALT));
    return chunk_;
}

void Compiler::emitByte(uint8_t byte) { chunk_->write(byte, 0); }
void Compiler::emitBytes(uint8_t b1, uint8_t b2) { emitByte(b1); emitByte(b2); }

int Compiler::addConstant(Value value) { return chunk_->addConstant(value); }

void Compiler::emitConstant(Value value) {
    emitBytes(static_cast<uint8_t>(OpCode::CONSTANT), static_cast<uint8_t>(addConstant(value)));
}

void Compiler::addLocal(const std::string& name) {
    locals_.push_back({name, scopeDepth});
}

int Compiler::resolveLocal(const std::string& name) {
    for (int i = locals_.size() - 1; i >= 0; i--) {
        if (locals_[i].name == name) return i;
    }
    return -1;
}

void Compiler::compileStmt(const StmtPtr& stmt) {
    std::visit([&](auto&& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Stmt::Print>) {
            compileExpr(node.expression);
            emitByte(static_cast<uint8_t>(OpCode::PRINT));
        } else if constexpr (std::is_same_v<T, Stmt::Expression>) {
            compileExpr(node.expression);
            emitByte(static_cast<uint8_t>(OpCode::POP));
        } else if constexpr (std::is_same_v<T, Stmt::Var>) {
             compileExpr(node.initializer);
             if (scopeDepth > 0) {
                 addLocal(node.name);
             } else {
                 emitBytes(static_cast<uint8_t>(OpCode::DEFINE_GLOBAL), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.name)))));
             }
        } else if constexpr (std::is_same_v<T, Stmt::If>) {
            compileExpr(node.condition);
            int thenJump = emitJump(OpCode::JUMP_IF_FALSE);
            emitByte(static_cast<uint8_t>(OpCode::POP));
            compileStmt(node.thenBranch);
            int elseJump = emitJump(OpCode::JUMP);
            patchJump(thenJump);
            emitByte(static_cast<uint8_t>(OpCode::POP));
            if (node.elseBranch) compileStmt(node.elseBranch);
            patchJump(elseJump);
        } else if constexpr (std::is_same_v<T, Stmt::While>) {
            int loopStart = chunk_->code.size();
            compileExpr(node.condition);
            int exitJump = emitJump(OpCode::JUMP_IF_FALSE);
            emitByte(static_cast<uint8_t>(OpCode::POP));
            compileStmt(node.body);
            emitLoop(loopStart);
            patchJump(exitJump);
            emitByte(static_cast<uint8_t>(OpCode::POP));
        } else if constexpr (std::is_same_v<T, Stmt::Block>) {
            scopeDepth++;
            for (const auto& s : node.statements) compileStmt(s);
            int locals_to_pop = 0;
            while (locals_.size() > 0 && locals_.back().depth == scopeDepth) {
                locals_to_pop++;
                locals_.pop_back();
            }
            scopeDepth--;
            for(int i=0; i<locals_to_pop; ++i) emitByte(static_cast<uint8_t>(OpCode::POP));
        }
    }, stmt->node);
}

int Compiler::emitJump(OpCode instruction) {
    emitByte(static_cast<uint8_t>(instruction));
    emitByte(0xff); emitByte(0xff);
    return chunk_->code.size() - 2;
}

void Compiler::patchJump(int offset) {
    int jump = chunk_->code.size() - offset - 2;
    chunk_->code[offset] = (jump >> 8) & 0xff;
    chunk_->code[offset + 1] = jump & 0xff;
}

void Compiler::emitLoop(int loopStart) {
    emitByte(static_cast<uint8_t>(OpCode::LOOP));
    int offset = chunk_->code.size() - loopStart + 2;
    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

void Compiler::compileExpr(const ExprPtr& expr) {
    std::visit([&](auto&& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Expr::Literal>) {
            if (std::holds_alternative<double>(node.value)) {
                emitConstant(doubleToValue(std::get<double>(node.value)));
            } else {
                std::string s = std::get<std::string>(node.value);
                if (s == "true") emitByte(static_cast<uint8_t>(OpCode::TRUE));
                else if (s == "false") emitByte(static_cast<uint8_t>(OpCode::FALSE));
                else emitConstant(objToValue(new ObjString(s)));
            }
        } else if constexpr (std::is_same_v<T, Expr::Binary>) {
            compileExpr(node.left);
            compileExpr(node.right);
            switch (node.op.type) {
                case TokenType::PLUS:  emitByte(static_cast<uint8_t>(OpCode::ADD)); break;
                case TokenType::MINUS: emitByte(static_cast<uint8_t>(OpCode::SUBTRACT)); break;
                case TokenType::STAR:  emitByte(static_cast<uint8_t>(OpCode::MULTIPLY)); break;
                case TokenType::SLASH: emitByte(static_cast<uint8_t>(OpCode::DIVIDE)); break;
                case TokenType::LESS:  emitByte(static_cast<uint8_t>(OpCode::LESS)); break;
                case TokenType::GREATER: emitByte(static_cast<uint8_t>(OpCode::GREATER)); break;
                case TokenType::EQUAL_EQUAL: emitByte(static_cast<uint8_t>(OpCode::EQUAL)); break;
                default: break;
            }
        } else if constexpr (std::is_same_v<T, Expr::Assign>) {
            compileExpr(node.value);
            int target = resolveLocal(node.name);
            if (target != -1) {
                emitBytes(static_cast<uint8_t>(OpCode::SET_LOCAL), (uint8_t)target);
            } else {
                emitBytes(static_cast<uint8_t>(OpCode::SET_GLOBAL), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.name)))));
            }
        } else if constexpr (std::is_same_v<T, Expr::Variable>) {
            int slot = resolveLocal(node.name);
            if (slot != -1) {
                emitBytes(static_cast<uint8_t>(OpCode::GET_LOCAL), (uint8_t)slot);
            } else {
                emitBytes(static_cast<uint8_t>(OpCode::GET_GLOBAL), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.name)))));
            }
        } else if constexpr (std::is_same_v<T, Expr::Grouping>) {
            compileExpr(node.expression);
        } else if constexpr (std::is_same_v<T, Expr::SysQuery>) {
            emitBytes(static_cast<uint8_t>(OpCode::SYS_QUERY), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.key)))));
        }
    }, expr->node);
}

} // namespace kio
