/* Copyright (c) 2025 Dipanjan Dhar SPDX-License-Identifier: GPL-3.0-only */
#pragma once

#include <vector>
#include <memory>
#include "axeon/token.hpp"
#include "axeon/ast.hpp"

namespace kio {

class Parser {
public:
    explicit Parser(const std::vector<Token> &tokens);
    std::vector<StmtPtr> parse();

private:
    const std::vector<Token> tokens_;
    size_t current_ {0};

    bool isAtEnd() const;
    const Token &peek() const;
    const Token &previous() const;
    const Token &advance();
    bool check(TokenType type) const;
    bool match(std::initializer_list<TokenType> types);

    StmtPtr declaration();
    StmtPtr varDeclaration();
    StmtPtr constDeclaration();
    StmtPtr functionDeclaration();
    StmtPtr statement();
    StmtPtr printStatement();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    StmtPtr forStatement();
    StmtPtr block();
    StmtPtr saveStatement();
    StmtPtr loadStatement();
    StmtPtr importStatement();
    StmtPtr expressionStatement();
    StmtPtr switchStatement();
    StmtPtr tryCatchStatement();
    StmtPtr throwStatement();
    StmtPtr breakStatement();
    StmtPtr continueStatement();
    StmtPtr returnStatement();
    StmtPtr classDeclaration();
    StmtPtr namespaceDeclaration();
    StmtPtr moduleDeclaration();
    StmtPtr parallelStatement();
    StmtPtr exportStatement();

    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr or_expr();
    ExprPtr and_expr();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr primary();
    ExprPtr call();
    ExprPtr finishCall(ExprPtr callee);
    ExprPtr array();
    ExprPtr ternary();
    ExprPtr lambda();

    ExprPtr sysQuery();
};

} // namespace kio
