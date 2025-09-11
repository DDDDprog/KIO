/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>

namespace kio {

enum class TokenType {
    // Single-char tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    PLUS, MINUS, STAR, SLASH, PERCENT,
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    SEMICOLON,

    // Literals
    IDENTIFIER,
    NUMBER,
    STRING,

    // Keywords
    PRINT,
    LET,
    SAVE,
    LOAD,
    SYS,
    IMPORT,
    IF,
    ELSE,
    WHILE,

    END_OF_FILE,
    INVALID
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

} // namespace kio
