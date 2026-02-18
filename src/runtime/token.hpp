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

    // Literals
    IDENTIFIER, STRING, NUMBER, CHAR, RAW_STRING,

    // Keywords
    LET, VAR, CONST, FN, FUNCTION, IF, ELSE, ELIF, THEN, END, FOR, WHILE, DO, RETURN,
    CLASS, STRUCT, ENUM, INTERFACE, TRAIT, IMPL, EXTENDS, IMPLEMENTS,
    SUPER, THIS, SELF, STATIC, PUB, PRIV, MUT, TYPE,
    NAMESPACE, USING, AS, IS, IN, NOT, AND, OR, WHERE, OF,
    ASYNC, AWAIT, YIELD, WITH, FINALLY, FROM,
    MODULE, EXPORT, PARALLEL,
    TRUE, FALSE, NIL,
    SYS, PRINT, SAVE, LOAD, IMPORT, USE,
    SWITCH, CASE, DEFAULT, BREAK, CONTINUE, MATCH,
    TRY, CATCH, THROW, NEW, DELETE, NULLABLE,
    PANIC, ASSERT, SIZEOF, TYPEOF, UNSAFE, EXTERN,

    // Storage Types
    T_I8, T_I16, T_I32, T_I64, T_I128,
    T_U8, T_U16, T_U32, T_U64, T_U128,
    T_F32, T_F64,
    T_BOOL, T_CHAR, T_STR, T_STRING,
    T_INT, T_FLOAT, T_VOID, T_ANY, T_TYPE, T_USIZE, T_ISIZE,

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
