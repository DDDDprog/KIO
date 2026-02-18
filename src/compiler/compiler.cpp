/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/compiler.hpp"
#include <iostream>

namespace kio {

Compiler::Compiler(Compiler* parent, FunctionType type) 
    : parent_(parent), type_(type) {
    function_ = new ObjFunction();
    if (type == FunctionType::TYPE_SCRIPT) {
        function_->name = "script";
    }
    
    // Reserve stack slot 0 for function itself (or 'this')
    locals_.push_back({"", 0});
}

ObjFunction* Compiler::compile(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) { compileStmt(stmt); }
    emitByte(static_cast<uint8_t>(OpCode::HALT));
    return function_;
}

void Compiler::emitByte(uint8_t byte) { currentChunk()->write(byte, 0); }
void Compiler::emitBytes(uint8_t b1, uint8_t b2) { emitByte(b1); emitByte(b2); }

int Compiler::addConstant(Value value) { return currentChunk()->addConstant(value); }

void Compiler::emitConstant(Value value) {
    int idx = addConstant(value);
    if (idx > 255) {
        // Handle large constant pool if needed, but for now 255 is limit
    }
    emitBytes(static_cast<uint8_t>(OpCode::CONSTANT), static_cast<uint8_t>(idx));
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
        } else if constexpr (std::is_same_v<T, Stmt::Function>) {
            Compiler sub(this, FunctionType::TYPE_FUNCTION);
            sub.function_->name = node.name;
            sub.function_->arity = node.params.size();
            sub.scopeDepth++;
            for (const auto& param : node.params) {
                sub.addLocal(param.first);
            }
            for (const auto& s : node.body) {
                sub.compileStmt(s);
            }
            sub.emitByte(static_cast<uint8_t>(OpCode::HALT));
            
            emitConstant(objToValue(sub.function_));
            if (scopeDepth > 0) {
                addLocal(node.name);
            } else {
                emitBytes(static_cast<uint8_t>(OpCode::DEFINE_GLOBAL), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.name)))));
            }
        } else if constexpr (std::is_same_v<T, Stmt::Return>) {
            if (type_ == FunctionType::TYPE_SCRIPT) {
                // Error actually, but emit halt/return nil for now
            }
            if (node.value) {
                compileExpr(node.value);
            } else {
                emitByte(static_cast<uint8_t>(OpCode::NIL));
            }
            emitByte(static_cast<uint8_t>(OpCode::RETURN));
        } else if constexpr (std::is_same_v<T, Stmt::Class>) {
            emitBytes(static_cast<uint8_t>(OpCode::CLASS), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.name)))));
            if (scopeDepth > 0) {
                addLocal(node.name);
            } else {
                emitBytes(static_cast<uint8_t>(OpCode::DEFINE_GLOBAL), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.name)))));
            }
            
            // Methods
            for (const auto& m : node.methods) {
                if (auto func = std::get_if<Stmt::Function>(&m->node)) {
                    Compiler sub(this, FunctionType::TYPE_FUNCTION);
                    sub.function_->name = func->name;
                    sub.function_->arity = func->params.size();
                    sub.scopeDepth++;
                    sub.addLocal("this"); // Implicit this
                    for (const auto& param : func->params) {
                        sub.addLocal(param.first);
                    }
                    for (const auto& s : func->body) {
                        sub.compileStmt(s);
                    }
                    sub.emitByte(static_cast<uint8_t>(OpCode::HALT));
                    emitConstant(objToValue(sub.function_));
                    emitBytes(static_cast<uint8_t>(OpCode::METHOD), static_cast<uint8_t>(addConstant(objToValue(new ObjString(func->name)))));
                }
            }
            emitByte(static_cast<uint8_t>(OpCode::POP)); // Pop class name
        } else if constexpr (std::is_same_v<T, Stmt::If>) {
            compileExpr(node.condition);
            int thenJump = emitJump(OpCode::JUMP_IF_FALSE);
            compileStmt(node.thenBranch);
            int elseJump = emitJump(OpCode::JUMP);
            patchJump(thenJump);
            if (node.elseBranch) compileStmt(node.elseBranch);
            patchJump(elseJump);
        } else if constexpr (std::is_same_v<T, Stmt::While>) {
            int loopStart = currentChunk()->code.size();
            compileExpr(node.condition);
            int exitJump = emitJump(OpCode::JUMP_IF_FALSE);
            compileStmt(node.body);
            emitLoop(loopStart);
            patchJump(exitJump);
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
        } else if constexpr (std::is_same_v<T, Stmt::ForIn>) {
            scopeDepth++;
            emitConstant(doubleToValue(0));
            addLocal(node.name);
            int loopVarSlot = locals_.size() - 1;
            compileExpr(node.iterable);
            addLocal("_limit");
            int limitVarSlot = locals_.size() - 1;
            int loopStart = currentChunk()->code.size();
            emitBytes(static_cast<uint8_t>(OpCode::GET_LOCAL), (uint8_t)loopVarSlot);
            emitBytes(static_cast<uint8_t>(OpCode::GET_LOCAL), (uint8_t)limitVarSlot);
            emitByte(static_cast<uint8_t>(OpCode::LESS));
            int exitJump = emitJump(OpCode::JUMP_IF_FALSE);
            compileStmt(node.body);
            emitBytes(static_cast<uint8_t>(OpCode::GET_LOCAL), (uint8_t)loopVarSlot);
            emitConstant(doubleToValue(1));
            emitByte(static_cast<uint8_t>(OpCode::ADD));
            emitBytes(static_cast<uint8_t>(OpCode::SET_LOCAL), (uint8_t)loopVarSlot);
            emitByte(static_cast<uint8_t>(OpCode::POP));
            emitLoop(loopStart);
            patchJump(exitJump);
            locals_.pop_back(); locals_.pop_back();
            scopeDepth--;
            emitByte(static_cast<uint8_t>(OpCode::POP)); emitByte(static_cast<uint8_t>(OpCode::POP));
        } else if constexpr (std::is_same_v<T, Stmt::For>) {
            scopeDepth++;
            if (node.initializer) compileStmt(node.initializer);
            int loopStart = currentChunk()->code.size();
            int exitJump = -1;
            if (node.condition) {
                compileExpr(node.condition);
                exitJump = emitJump(OpCode::JUMP_IF_FALSE);
            }
            compileStmt(node.body);
            if (node.increment) {
                compileExpr(node.increment);
                emitByte(static_cast<uint8_t>(OpCode::POP));
            }
            emitLoop(loopStart);
            if (exitJump != -1) patchJump(exitJump);
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
    return currentChunk()->code.size() - 2;
}

void Compiler::patchJump(int offset) {
    int jump = currentChunk()->code.size() - offset - 2;
    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

void Compiler::emitLoop(int loopStart) {
    emitByte(static_cast<uint8_t>(OpCode::LOOP));
    int offset = currentChunk()->code.size() - loopStart + 2;
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
                else if (s == "") emitByte(static_cast<uint8_t>(OpCode::NIL));
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
                case TokenType::PERCENT: emitByte(static_cast<uint8_t>(OpCode::MODULO)); break;
                case TokenType::LESS:  emitByte(static_cast<uint8_t>(OpCode::LESS)); break;
                case TokenType::LESS_EQUAL: emitByte(static_cast<uint8_t>(OpCode::LESS_EQUAL)); break;
                case TokenType::GREATER: emitByte(static_cast<uint8_t>(OpCode::GREATER)); break;
                case TokenType::GREATER_EQUAL: emitByte(static_cast<uint8_t>(OpCode::GREATER_EQUAL)); break;
                case TokenType::EQUAL_EQUAL: emitByte(static_cast<uint8_t>(OpCode::EQUAL)); break;
                case TokenType::BANG_EQUAL: {
                    emitByte(static_cast<uint8_t>(OpCode::EQUAL));
                    emitByte(static_cast<uint8_t>(OpCode::NOT));
                    break;
                }
                default: 
                    std::cerr << "Unknown operator in compiler: " << (int)node.op.type << std::endl;
                    break;
            }
        } else if constexpr (std::is_same_v<T, Expr::Assign>) {
            compileExpr(node.value);
            int target = resolveLocal(node.name);
            if (target != -1) {
                emitBytes(static_cast<uint8_t>(OpCode::SET_LOCAL), (uint8_t)target);
            } else {
                emitBytes(static_cast<uint8_t>(OpCode::SET_GLOBAL), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.name)))));
            }
        } else if constexpr (std::is_same_v<T, Expr::Call>) {
            if (std::holds_alternative<Expr::Variable>(node.callee->node)) {
                auto& var = std::get<Expr::Variable>(node.callee->node);
                if (var.name == "floor") { compileExpr(node.arguments[0]); emitByte(static_cast<uint8_t>(OpCode::FLOOR)); return; }
                if (var.name == "sqrt") { compileExpr(node.arguments[0]); emitByte(static_cast<uint8_t>(OpCode::SQRT)); return; }
            }
            for (const auto& arg : node.arguments) compileExpr(arg);
            compileExpr(node.callee);
            emitBytes(static_cast<uint8_t>(OpCode::CALL), (uint8_t)node.arguments.size());
        } else if constexpr (std::is_same_v<T, Expr::Get>) {
            compileExpr(node.object);
            emitBytes(static_cast<uint8_t>(OpCode::GET_PROPERTY), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.name)))));
        } else if constexpr (std::is_same_v<T, Expr::Set>) {
            compileExpr(node.object);
            compileExpr(node.value);
            emitBytes(static_cast<uint8_t>(OpCode::SET_PROPERTY), static_cast<uint8_t>(addConstant(objToValue(new ObjString(node.name)))));
        } else if constexpr (std::is_same_v<T, Expr::This>) {
            int slot = resolveLocal("this");
            if (slot != -1) emitBytes(static_cast<uint8_t>(OpCode::GET_LOCAL), (uint8_t)slot);
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
        } else if constexpr (std::is_same_v<T, Expr::Array>) {
            for (const auto& element : node.elements) compileExpr(element);
            emitBytes(static_cast<uint8_t>(OpCode::ARRAY_NEW), (uint8_t)node.elements.size());
        } else if constexpr (std::is_same_v<T, Expr::Index>) {
            compileExpr(node.object);
            compileExpr(node.index);
            emitByte(static_cast<uint8_t>(OpCode::ARRAY_GET));
        } else if constexpr (std::is_same_v<T, Expr::IndexSet>) {
            compileExpr(node.object);
            compileExpr(node.index);
            compileExpr(node.value);
            emitByte(static_cast<uint8_t>(OpCode::ARRAY_SET));
        }
    }, expr->node);
}

} // namespace kio
