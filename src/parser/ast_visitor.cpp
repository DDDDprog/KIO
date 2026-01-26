/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/ast_visitor.hpp"
#include "kio/ast.hpp"

namespace kio {

void ASTVisitor::visitStatement(const StmtPtr& stmt) {
    if (!stmt) return;
    
    std::visit([this](auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Stmt::Print>) {
            visitPrint(node);
        } else if constexpr (std::is_same_v<T, Stmt::Var>) {
            visitVar(node);
        } else if constexpr (std::is_same_v<T, Stmt::Expression>) {
            visitExpression(node);
        } else if constexpr (std::is_same_v<T, Stmt::Block>) {
            visitBlock(node);
        } else if constexpr (std::is_same_v<T, Stmt::If>) {
            visitIf(node);
        } else if constexpr (std::is_same_v<T, Stmt::While>) {
            visitWhile(node);
        } else if constexpr (std::is_same_v<T, Stmt::For>) {
            visitFor(node);
        } else if constexpr (std::is_same_v<T, Stmt::Function>) {
            visitFunction(node);
        } else if constexpr (std::is_same_v<T, Stmt::Switch>) {
            visitSwitch(node);
        } else if constexpr (std::is_same_v<T, Stmt::TryCatch>) {
            visitTryCatch(node);
        } else if constexpr (std::is_same_v<T, Stmt::Class>) {
            visitClass(node);
        } else if constexpr (std::is_same_v<T, Stmt::Namespace>) {
            visitNamespace(node);
        }
    }, stmt->node);
}

void ASTVisitor::visitExpression(const ExprPtr& expr) {
    if (!expr) return;
    
    std::visit([this](auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Expr::Literal>) {
            visitLiteral(node);
        } else if constexpr (std::is_same_v<T, Expr::Variable>) {
            visitVariable(node);
        } else if constexpr (std::is_same_v<T, Expr::Binary>) {
            visitBinary(node);
        } else if constexpr (std::is_same_v<T, Expr::Unary>) {
            visitUnary(node);
        } else if constexpr (std::is_same_v<T, Expr::Grouping>) {
            visitGrouping(node);
        } else if constexpr (std::is_same_v<T, Expr::Assign>) {
            visitAssign(node);
        } else if constexpr (std::is_same_v<T, Expr::Call>) {
            visitCall(node);
        } else if constexpr (std::is_same_v<T, Expr::Get>) {
            visitGet(node);
        } else if constexpr (std::is_same_v<T, Expr::Set>) {
            visitSet(node);
        } else if constexpr (std::is_same_v<T, Expr::Logical>) {
            visitLogical(node);
        } else if constexpr (std::is_same_v<T, Expr::Ternary>) {
            visitTernary(node);
        } else if constexpr (std::is_same_v<T, Expr::Array>) {
            visitArray(node);
        } else if constexpr (std::is_same_v<T, Expr::Index>) {
            visitIndex(node);
        } else if constexpr (std::is_same_v<T, Expr::Lambda>) {
            visitLambda(node);
        }
    }, expr->node);
}

// Default implementations (can be overridden)
void ASTVisitor::visitPrint(const Stmt::Print& stmt) {
    visitExpression(stmt.expression);
}

void ASTVisitor::visitVar(const Stmt::Var& stmt) {
    if (stmt.initializer) {
        visitExpression(stmt.initializer);
    }
}

void ASTVisitor::visitExpression(const Stmt::Expression& stmt) {
    visitExpression(stmt.expression);
}

void ASTVisitor::visitBlock(const Stmt::Block& stmt) {
    for (const auto& s : stmt.statements) {
        visitStatement(s);
    }
}

void ASTVisitor::visitIf(const Stmt::If& stmt) {
    visitExpression(stmt.condition);
    visitStatement(stmt.thenBranch);
    if (stmt.elseBranch) {
        visitStatement(stmt.elseBranch);
    }
}

void ASTVisitor::visitWhile(const Stmt::While& stmt) {
    visitExpression(stmt.condition);
    visitStatement(stmt.body);
}

void ASTVisitor::visitFor(const Stmt::For& stmt) {
    if (stmt.initializer) {
        visitStatement(stmt.initializer);
    }
    if (stmt.condition) {
        visitExpression(stmt.condition);
    }
    if (stmt.increment) {
        visitExpression(stmt.increment);
    }
    visitStatement(stmt.body);
}

void ASTVisitor::visitFunction(const Stmt::Function& stmt) {
    for (const auto& stmt : stmt.body) {
        visitStatement(stmt);
    }
}

void ASTVisitor::visitSwitch(const Stmt::Switch& stmt) {
    visitExpression(stmt.expression);
    for (const auto& [caseExpr, caseStmts] : stmt.cases) {
        visitExpression(caseExpr);
        for (const auto& s : caseStmts) {
            visitStatement(s);
        }
    }
    for (const auto& s : stmt.defaultCase) {
        visitStatement(s);
    }
}

void ASTVisitor::visitTryCatch(const Stmt::TryCatch& stmt) {
    for (const auto& s : stmt.tryBlock) {
        visitStatement(s);
    }
    for (const auto& s : stmt.catchBlock) {
        visitStatement(s);
    }
    for (const auto& s : stmt.finallyBlock) {
        visitStatement(s);
    }
}

void ASTVisitor::visitClass(const Stmt::Class& stmt) {
    for (const auto& m : stmt.methods) {
        visitStatement(m);
    }
    for (const auto& f : stmt.fields) {
        visitStatement(f);
    }
}

void ASTVisitor::visitNamespace(const Stmt::Namespace& stmt) {
    for (const auto& s : stmt.statements) {
        visitStatement(s);
    }
}

void ASTVisitor::visitLiteral(const Expr::Literal& expr) {
    // Leaf node, nothing to visit
}

void ASTVisitor::visitVariable(const Expr::Variable& expr) {
    // Leaf node, nothing to visit
}

void ASTVisitor::visitBinary(const Expr::Binary& expr) {
    visitExpression(expr.left);
    visitExpression(expr.right);
}

void ASTVisitor::visitUnary(const Expr::Unary& expr) {
    visitExpression(expr.right);
}

void ASTVisitor::visitGrouping(const Expr::Grouping& expr) {
    visitExpression(expr.expression);
}

void ASTVisitor::visitAssign(const Expr::Assign& expr) {
    visitExpression(expr.value);
}

void ASTVisitor::visitCall(const Expr::Call& expr) {
    visitExpression(expr.callee);
    for (const auto& arg : expr.arguments) {
        visitExpression(arg);
    }
}

void ASTVisitor::visitGet(const Expr::Get& expr) {
    visitExpression(expr.object);
}

void ASTVisitor::visitSet(const Expr::Set& expr) {
    visitExpression(expr.object);
    visitExpression(expr.value);
}

void ASTVisitor::visitLogical(const Expr::Logical& expr) {
    visitExpression(expr.left);
    visitExpression(expr.right);
}

void ASTVisitor::visitTernary(const Expr::Ternary& expr) {
    visitExpression(expr.condition);
    visitExpression(expr.thenExpr);
    visitExpression(expr.elseExpr);
}

void ASTVisitor::visitArray(const Expr::Array& expr) {
    for (const auto& elem : expr.elements) {
        visitExpression(elem);
    }
}

void ASTVisitor::visitIndex(const Expr::Index& expr) {
    visitExpression(expr.object);
    visitExpression(expr.index);
}

void ASTVisitor::visitLambda(const Expr::Lambda& expr) {
    for (const auto& stmt : expr.body) {
        visitStatement(stmt);
    }
}

} // namespace kio
