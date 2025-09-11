#pragma once

#include <string>

namespace kio {

enum class TokenType {
    // Single-char tokens
    LEFT_PAREN, RIGHT_PAREN,
    PLUS, MINUS, STAR, SLASH,
    EQUAL,
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
