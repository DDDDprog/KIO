/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include "kio/token.hpp"

namespace kio {

class Lexer {
public:
    explicit Lexer(const std::string &source);
    std::vector<Token> scanTokens();
    std::vector<Token> tokenize();

private:
    const std::string source_;
    size_t start_ {0};
    size_t current_ {0};
    int line_ {1};
    int column_ {1};

    bool isAtEnd() const;
    void scanToken(std::vector<Token> &tokens);
    char advance();
    bool match(char expected);
    char peek() const;
    char peekNext() const;
    void addToken(std::vector<Token> &tokens, TokenType type);
    void identifier(std::vector<Token> &tokens);
    void number(std::vector<Token> &tokens);
    void stringLiteral(std::vector<Token> &tokens);
};

} // namespace kio
