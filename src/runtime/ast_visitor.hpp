/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include "axeon/ast.hpp"
#include <type_traits>

namespace kio {

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    void visitStatement(const StmtPtr& stmt);
    void visitExpression(const ExprPtr& expr);
    
    // Statement visitors
    virtual void visitPrint(const Stmt::Print& stmt);
    virtual void visitVar(const Stmt::Var& stmt);
    virtual void visitExpression(const Stmt::Expression& stmt);
    virtual void visitBlock(const Stmt::Block& stmt);
    virtual void visitIf(const Stmt::If& stmt);
    virtual void visitWhile(const Stmt::While& stmt);
    virtual void visitFor(const Stmt::For& stmt);
    virtual void visitFunction(const Stmt::Function& stmt);
    virtual void visitSwitch(const Stmt::Switch& stmt);
    virtual void visitTryCatch(const Stmt::TryCatch& stmt);
    virtual void visitClass(const Stmt::Class& stmt);
    virtual void visitNamespace(const Stmt::Namespace& stmt);
    
    // Expression visitors
    virtual void visitLiteral(const Expr::Literal& expr);
    virtual void visitVariable(const Expr::Variable& expr);
    virtual void visitBinary(const Expr::Binary& expr);
    virtual void visitUnary(const Expr::Unary& expr);
    virtual void visitGrouping(const Expr::Grouping& expr);
    virtual void visitAssign(const Expr::Assign& expr);
    virtual void visitCall(const Expr::Call& expr);
    virtual void visitGet(const Expr::Get& expr);
    virtual void visitSet(const Expr::Set& expr);
    virtual void visitLogical(const Expr::Logical& expr);
    virtual void visitTernary(const Expr::Ternary& expr);
    virtual void visitArray(const Expr::Array& expr);
    virtual void visitIndex(const Expr::Index& expr);
    virtual void visitLambda(const Expr::Lambda& expr);
};

} // namespace kio
