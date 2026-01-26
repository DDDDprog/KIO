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

struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

struct Expr {
    struct Literal { std::variant<double, std::string> value; };
    struct Variable { std::string name; };
    struct Binary { ExprPtr left; Token op; ExprPtr right; };
    struct Assign { std::string name; ExprPtr value; };
    struct Grouping { ExprPtr expression; };
    struct SysQuery { std::string key; };
    struct Logical { ExprPtr left; Token op; ExprPtr right; };
    struct PostOp { std::string name; Token op; };
    struct Call { ExprPtr callee; std::vector<ExprPtr> arguments; };
    struct Get { ExprPtr object; std::string name; };
    struct Set { ExprPtr object; std::string name; ExprPtr value; };
    struct This { std::string keyword; };
    struct Super { Token keyword; std::string method; };
    struct Unary { Token op; ExprPtr right; };
    struct Ternary { ExprPtr condition; ExprPtr thenExpr; ExprPtr elseExpr; };
    struct Array { std::vector<ExprPtr> elements; };
    struct Index { ExprPtr object; ExprPtr index; };
    struct IndexSet { ExprPtr object; ExprPtr index; ExprPtr value; };
    struct Lambda { std::vector<std::pair<std::string, std::string>> params; std::vector<StmtPtr> body; };

    std::variant<Literal, Variable, Binary, Grouping, Assign, SysQuery, Logical, PostOp, Call, Get, Set, This, Super, Unary, Ternary, Array, Index, IndexSet, Lambda> node;
};

struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;
struct Stmt {
    struct Print { ExprPtr expression; };
    struct Var { std::string name; ExprPtr initializer; std::string typeAnnotation; bool isConst; };
    struct Expression { ExprPtr expression; };
    struct Save { std::string path; };
    struct Load { std::string path; };
    struct Import { std::string path; };
    struct Block { std::vector<StmtPtr> statements; };
    struct If { ExprPtr condition; StmtPtr thenBranch; StmtPtr elseBranch; }; // elseBranch can be nullptr
    struct While { ExprPtr condition; StmtPtr body; };
    struct For { StmtPtr initializer; ExprPtr condition; ExprPtr increment; StmtPtr body; };
    struct ForIn { std::string name; ExprPtr iterable; StmtPtr body; };
    struct Function { std::string name; std::vector<std::pair<std::string, std::string>> params; std::string returnType; std::vector<StmtPtr> body; };
    struct Switch { ExprPtr expression; std::vector<std::pair<ExprPtr, std::vector<StmtPtr>>> cases; std::vector<StmtPtr> defaultCase; };
    struct TryCatch { std::vector<StmtPtr> tryBlock; std::string catchVar; std::vector<StmtPtr> catchBlock; std::vector<StmtPtr> finallyBlock; };
    struct Throw { ExprPtr expression; };
    struct Break {};
    struct Continue {};
    struct Return { ExprPtr value; };
    struct Class { std::string name; std::string superclass; std::vector<StmtPtr> methods; std::vector<StmtPtr> fields; };
    struct Namespace { std::string name; std::vector<StmtPtr> statements; };
    struct Parallel { std::vector<StmtPtr> body; };
    struct Module { std::string name; std::vector<StmtPtr> body; };
    struct Export { StmtPtr statement; };
    struct ImportFrom { std::string name; std::string path; };

    std::variant<Print, Var, Expression, Save, Load, Import, Block, If, While, For, ForIn, Function, Switch, TryCatch, Throw, Break, Continue, Return, Class, Namespace, Parallel, Module, Export, ImportFrom> node;
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
