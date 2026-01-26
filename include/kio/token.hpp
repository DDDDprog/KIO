/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <map>

namespace kio {

enum class TokenType {
    // Single-char tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,
    PLUS, MINUS, STAR, SLASH, PERCENT,
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    DOT, COMMA, SEMICOLON, COLON,
    QUESTION,

    // Two+ char tokens
    PLUS_PLUS, MINUS_MINUS,
    PLUS_EQUAL, MINUS_EQUAL,
    STAR_EQUAL, SLASH_EQUAL,
    ARROW,
    AND, OR,

    // Literals
    IDENTIFIER, STRING, NUMBER, CHAR, RAW_STRING,

    // Keywords
    LET, CONST, FUNCTION, IF, ELSE, FOR, WHILE, RETURN,
    CLASS, SUPER, THIS, STATIC, PUBLIC, PRIVATE, PROTECTED,
    INTERFACE, ENUM, NAMESPACE, USING, AS, IS,
    ASYNC, AWAIT, YIELD, WITH, FINALLY, FROM,
    MODULE, EXPORT, PARALLEL, IN,
    TRUE, FALSE, NIL,
    SYS, PRINT, SAVE, LOAD, IMPORT,
    SWITCH, CASE, DEFAULT, BREAK, CONTINUE,
    TRY, CATCH, THROW, NEW, NULLABLE,

    END_OF_FILE,
    INVALID
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

std::string tokenTypeToString(TokenType type);

} // namespace kio
