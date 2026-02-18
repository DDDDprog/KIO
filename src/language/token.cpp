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
        case TokenType::VAR: return "VAR";
        case TokenType::CONST: return "CONST";
        case TokenType::FN: return "FN";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::SAVE: return "SAVE";
        case TokenType::LOAD: return "LOAD";
        case TokenType::SYS: return "SYS";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::USE: return "USE";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::ELIF: return "ELIF";
        case TokenType::THEN: return "THEN";
        case TokenType::END: return "END";
        case TokenType::WHILE: return "WHILE";
        case TokenType::DO: return "DO";
        case TokenType::FOR: return "FOR";
        case TokenType::RETURN: return "RETURN";
        case TokenType::SWITCH: return "SWITCH";
        case TokenType::CASE: return "CASE";
        case TokenType::DEFAULT: return "DEFAULT";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::MATCH: return "MATCH";
        case TokenType::TRY: return "TRY";
        case TokenType::CATCH: return "CATCH";
        case TokenType::THROW: return "THROW";
        case TokenType::CLASS: return "CLASS";
        case TokenType::STRUCT: return "STRUCT";
        case TokenType::ENUM: return "ENUM";
        case TokenType::INTERFACE: return "INTERFACE";
        case TokenType::TRAIT: return "TRAIT";
        case TokenType::IMPL: return "IMPL";
        case TokenType::EXTENDS: return "EXTENDS";
        case TokenType::IMPLEMENTS: return "IMPLEMENTS";
        case TokenType::NEW: return "NEW";
        case TokenType::DELETE: return "DELETE";
        case TokenType::THIS: return "THIS";
        case TokenType::SUPER: return "SUPER";
        case TokenType::SELF: return "SELF";
        case TokenType::STATIC: return "STATIC";
        case TokenType::PUB: return "PUB";
        case TokenType::PRIV: return "PRIV";
        case TokenType::MUT: return "MUT";
        case TokenType::TYPE: return "TYPE";
        case TokenType::NAMESPACE: return "NAMESPACE";
        case TokenType::USING: return "USING";
        case TokenType::AS: return "AS";
        case TokenType::IS: return "IS";
        case TokenType::IN: return "IN";
        case TokenType::NOT: return "NOT";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::WHERE: return "WHERE";
        case TokenType::OF: return "OF";
        case TokenType::NULLABLE: return "NULLABLE";
        case TokenType::ASYNC: return "ASYNC";
        case TokenType::AWAIT: return "AWAIT";
        case TokenType::YIELD: return "YIELD";
        case TokenType::WITH: return "WITH";
        case TokenType::FINALLY: return "FINALLY";
        case TokenType::FROM: return "FROM";
        case TokenType::MODULE: return "MODULE";
        case TokenType::EXPORT: return "EXPORT";
        case TokenType::PARALLEL: return "PARALLEL";
        case TokenType::PANIC: return "PANIC";
        case TokenType::ASSERT: return "ASSERT";
        case TokenType::SIZEOF: return "SIZEOF";
        case TokenType::TYPEOF: return "TYPEOF";
        case TokenType::UNSAFE: return "UNSAFE";
        case TokenType::EXTERN: return "EXTERN";
        case TokenType::CHAR: return "CHAR";
        case TokenType::RAW_STRING: return "RAW_STRING";
        case TokenType::QUESTION: return "QUESTION";
        case TokenType::DOT: return "DOT";
        case TokenType::T_I8: return "I8";
        case TokenType::T_I16: return "I16";
        case TokenType::T_I32: return "I32";
        case TokenType::T_I64: return "I64";
        case TokenType::T_I128: return "I128";
        case TokenType::T_U8: return "U8";
        case TokenType::T_U16: return "U16";
        case TokenType::T_U32: return "U32";
        case TokenType::T_U64: return "U64";
        case TokenType::T_U128: return "U128";
        case TokenType::T_F32: return "F32";
        case TokenType::T_F64: return "F64";
        case TokenType::T_BOOL: return "BOOL";
        case TokenType::T_CHAR: return "CHAR_TYPE";
        case TokenType::T_STR: return "STR";
        case TokenType::T_STRING: return "STRING_TYPE";
        case TokenType::T_INT: return "INT";
        case TokenType::T_FLOAT: return "FLOAT";
        case TokenType::T_VOID: return "VOID";
        case TokenType::T_ANY: return "ANY";
        case TokenType::T_TYPE: return "TYPE_LITERAL";
        case TokenType::T_USIZE: return "USIZE";
        case TokenType::T_ISIZE: return "ISIZE";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::INVALID: return "INVALID";
    }
    return "UNKNOWN";
}

bool isKeyword(TokenType type) {
    return (type >= TokenType::LET && type <= TokenType::EXTERN);
}

bool isLiteral(TokenType type) {
    return type == TokenType::NUMBER || type == TokenType::STRING ||
           type == TokenType::TRUE || type == TokenType::FALSE ||
           type == TokenType::NIL;
}

bool isOperator(TokenType type) {
    return (type >= TokenType::BANG && type <= TokenType::OR) ||
           (type >= TokenType::PLUS_PLUS && type <= TokenType::SLASH_EQUAL) ||
           type == TokenType::DOT || type == TokenType::QUESTION;
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
