/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include "kio/token.hpp"

namespace kio {

class ASTNode {
public:
    virtual ~ASTNode() = default;
    std::vector<std::shared_ptr<ASTNode>> children;
    
    enum class Type {
        Expression,
        Statement,
        Literal,
        Variable,
        Binary,
        Assignment,
        Print,
        Declaration
    };
    
    virtual Type getType() const = 0;
    virtual std::string toString() const = 0;
};

struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct Expr {
    struct Literal { std::variant<double, std::string> value; };
    struct Variable { std::string name; };
    struct Binary { ExprPtr left; Token op; ExprPtr right; };
    struct Assign { std::string name; ExprPtr value; };
    struct Grouping { ExprPtr expression; };
    struct SysQuery { std::string key; };

    std::variant<Literal, Variable, Binary, Grouping, Assign, SysQuery> node;
};

struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

struct Stmt {
    struct Print { ExprPtr expression; };
    struct Var { std::string name; ExprPtr initializer; };
    struct Expression { ExprPtr expression; };
    struct Save { std::string path; };
    struct Load { std::string path; };
    struct Import { std::string path; };

    std::variant<Print, Var, Expression, Save, Load, Import> node;
};

class ExpressionNode : public ASTNode {
public:
    ExprPtr expression;
    
    ExpressionNode(ExprPtr expr) : expression(std::move(expr)) {}
    
    Type getType() const override { return Type::Expression; }
    std::string toString() const override { return "Expression"; }
};

class StatementNode : public ASTNode {
public:
    StmtPtr statement;
    
    StatementNode(StmtPtr stmt) : statement(std::move(stmt)) {}
    
    Type getType() const override { return Type::Statement; }
    std::string toString() const override { return "Statement"; }
};

} // namespace kio
