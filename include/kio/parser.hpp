/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <vector>
#include <memory>
#include "kio/token.hpp"
#include "kio/ast.hpp"

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
    StmtPtr statement();
    StmtPtr printStatement();
    StmtPtr saveStatement();
    StmtPtr loadStatement();
    StmtPtr importStatement();
    StmtPtr expressionStatement();

    ExprPtr expression();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr primary();

    ExprPtr assignment();

    ExprPtr sysQuery();
};

} // namespace kio
