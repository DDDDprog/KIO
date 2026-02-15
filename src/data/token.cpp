/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/token.hpp"
#include <sstream>
#include <iomanip>

namespace kio {

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LEFT_PAREN: return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE: return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
        case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TokenType::ARROW: return "ARROW";
        case TokenType::FROM: return "FROM";
        case TokenType::MODULE: return "MODULE";
        case TokenType::EXPORT: return "EXPORT";
        case TokenType::PARALLEL: return "PARALLEL";
        case TokenType::IN: return "IN";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::BANG: return "BANG";
        case TokenType::BANG_EQUAL: return "BANG_EQUAL";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COLON: return "COLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::PLUS_PLUS: return "PLUS_PLUS";
        case TokenType::MINUS_MINUS: return "MINUS_MINUS";
        case TokenType::PLUS_EQUAL: return "PLUS_EQUAL";
        case TokenType::MINUS_EQUAL: return "MINUS_EQUAL";
        case TokenType::STAR_EQUAL: return "STAR_EQUAL";
        case TokenType::SLASH_EQUAL: return "SLASH_EQUAL";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::NIL: return "NIL";
        case TokenType::PRINT: return "PRINT";
        case TokenType::LET: return "LET";
        case TokenType::CONST: return "CONST";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::SAVE: return "SAVE";
        case TokenType::LOAD: return "LOAD";
        case TokenType::SYS: return "SYS";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::RETURN: return "RETURN";
        case TokenType::SWITCH: return "SWITCH";
        case TokenType::CASE: return "CASE";
        case TokenType::DEFAULT: return "DEFAULT";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::TRY: return "TRY";
        case TokenType::CATCH: return "CATCH";
        case TokenType::THROW: return "THROW";
        case TokenType::CLASS: return "CLASS";
        case TokenType::NEW: return "NEW";
        case TokenType::THIS: return "THIS";
        case TokenType::SUPER: return "SUPER";
        case TokenType::STATIC: return "STATIC";
        case TokenType::PUBLIC: return "PUBLIC";
        case TokenType::PRIVATE: return "PRIVATE";
        case TokenType::PROTECTED: return "PROTECTED";
        case TokenType::INTERFACE: return "INTERFACE";
        case TokenType::ENUM: return "ENUM";
        case TokenType::NAMESPACE: return "NAMESPACE";
        case TokenType::USING: return "USING";
        case TokenType::AS: return "AS";
        case TokenType::IS: return "IS";
        case TokenType::NULLABLE: return "NULLABLE";
        case TokenType::ASYNC: return "ASYNC";
        case TokenType::AWAIT: return "AWAIT";
        case TokenType::YIELD: return "YIELD";
        case TokenType::WITH: return "WITH";
        case TokenType::FINALLY: return "FINALLY";
        case TokenType::CHAR: return "CHAR";
        case TokenType::RAW_STRING: return "RAW_STRING";
        case TokenType::QUESTION: return "QUESTION";
        case TokenType::DOT: return "DOT";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::INVALID: return "INVALID";
    }
    return "UNKNOWN";
}

bool isKeyword(TokenType type) {
    return type == TokenType::PRINT || type == TokenType::LET || 
           type == TokenType::CONST || type == TokenType::FUNCTION ||
           type == TokenType::SAVE || type == TokenType::LOAD ||
           type == TokenType::SYS || type == TokenType::IMPORT ||
           type == TokenType::IF || type == TokenType::ELSE ||
           type == TokenType::WHILE || type == TokenType::FOR ||
           type == TokenType::RETURN || type == TokenType::TRUE ||
           type == TokenType::FALSE || type == TokenType::NIL;
}

bool isLiteral(TokenType type) {
    return type == TokenType::NUMBER || type == TokenType::STRING ||
           type == TokenType::TRUE || type == TokenType::FALSE ||
           type == TokenType::NIL;
}

bool isOperator(TokenType type) {
    return type == TokenType::PLUS || type == TokenType::MINUS ||
           type == TokenType::STAR || type == TokenType::SLASH ||
           type == TokenType::PERCENT || type == TokenType::BANG ||
           type == TokenType::BANG_EQUAL || type == TokenType::EQUAL_EQUAL ||
           type == TokenType::GREATER || type == TokenType::GREATER_EQUAL ||
           type == TokenType::LESS || type == TokenType::LESS_EQUAL ||
           type == TokenType::AND || type == TokenType::OR ||
           type == TokenType::PLUS_PLUS || type == TokenType::MINUS_MINUS ||
           type == TokenType::PLUS_EQUAL || type == TokenType::MINUS_EQUAL ||
           type == TokenType::STAR_EQUAL || type == TokenType::SLASH_EQUAL;
}

bool isBinaryOperator(TokenType type) {
    return (type == TokenType::PLUS || type == TokenType::MINUS ||
            type == TokenType::STAR || type == TokenType::SLASH ||
            type == TokenType::PERCENT || type == TokenType::EQUAL_EQUAL ||
            type == TokenType::BANG_EQUAL || type == TokenType::GREATER ||
            type == TokenType::GREATER_EQUAL || type == TokenType::LESS ||
            type == TokenType::LESS_EQUAL || type == TokenType::AND ||
            type == TokenType::OR);
}

int getOperatorPrecedence(TokenType type) {
    switch (type) {
        case TokenType::OR: return 1;
        case TokenType::AND: return 2;
        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL: return 3;
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::LESS:
        case TokenType::LESS_EQUAL: return 4;
        case TokenType::PLUS:
        case TokenType::MINUS: return 5;
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT: return 6;
        default: return 0;
    }
}

std::string tokenToString(const Token& token) {
    std::ostringstream oss;
    oss << tokenTypeToString(token.type);
    if (!token.lexeme.empty() && token.type != TokenType::END_OF_FILE) {
        oss << "(" << token.lexeme << ")";
    }
    oss << " at " << token.line << ":" << token.column;
    return oss.str();
}

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.lexeme == rhs.lexeme &&
           lhs.line == rhs.line && lhs.column == rhs.column;
}

bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs == rhs);
}

} // namespace kio
