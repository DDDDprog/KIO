/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/lexer.hpp"
#include <cctype>
#include <unordered_map>
#include "axeon/config.hpp"

namespace kio {

static bool isAlpha(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
static bool isAlphaNumeric(char c) { return isAlpha(c) || std::isdigit(static_cast<unsigned char>(c)); }

Lexer::Lexer(const std::string &source) : source_(source) {}

std::vector<Token> Lexer::scanTokens() {
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        start_ = current_;
        startColumn_ = column_;
        scanToken(tokens);
    }
    tokens.push_back({TokenType::END_OF_FILE, "", line_, column_});
    return tokens;
}

std::vector<Token> Lexer::tokenize() {
    return scanTokens();
}

bool Lexer::isAtEnd() const { return current_ >= source_.size(); }

char Lexer::advance() {
    char c = source_[current_++];
    column_++;
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[current_] != expected) return false;
    current_++;
    column_++;
    return true;
}

char Lexer::peek() const { return isAtEnd() ? '\0' : source_[current_]; }
char Lexer::peekNext() const { return (current_ + 1 >= source_.size()) ? '\0' : source_[current_ + 1]; }

void Lexer::addToken(std::vector<Token> &tokens, TokenType type) {
    tokens.push_back({type, source_.substr(start_, current_ - start_), line_, startColumn_});
}

void Lexer::identifier(std::vector<Token> &tokens) {
    while (isAlphaNumeric(peek())) advance();
    std::string text = source_.substr(start_, current_ - start_);
    static std::unordered_map<std::string, TokenType> keywords{
        // Control flow
        {"if", TokenType::IF}, {"elif", TokenType::ELIF}, {"else", TokenType::ELSE},
        {"then", TokenType::THEN}, {"end", TokenType::END}, {"for", TokenType::FOR},
        {"while", TokenType::WHILE}, {"do", TokenType::DO}, {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE}, {"return", TokenType::RETURN},
        {"match", TokenType::MATCH}, {"case", TokenType::CASE}, {"switch", TokenType::SWITCH},
        {"default", TokenType::DEFAULT},
        // Import/Export
        {"import", TokenType::IMPORT}, {"export", TokenType::EXPORT},
        {"module", TokenType::MODULE}, {"as", TokenType::AS}, {"from", TokenType::FROM},
        {"use", TokenType::USE},
        // Functions
        {"fn", TokenType::FN}, {"function", TokenType::FUNCTION}, {"async", TokenType::ASYNC},
        {"await", TokenType::AWAIT}, {"yield", TokenType::YIELD}, {"throw", TokenType::THROW},
        {"try", TokenType::TRY}, {"catch", TokenType::CATCH}, {"finally", TokenType::FINALLY},
        // Types/Classes
        {"class", TokenType::CLASS}, {"struct", TokenType::STRUCT}, {"enum", TokenType::ENUM},
        {"interface", TokenType::INTERFACE}, {"trait", TokenType::TRAIT}, {"impl", TokenType::IMPL},
        {"extends", TokenType::EXTENDS}, {"implements", TokenType::IMPLEMENTS},
        {"pub", TokenType::PUB}, {"priv", TokenType::PRIV}, {"static", TokenType::STATIC},
        {"const", TokenType::CONST}, {"let", TokenType::LET}, {"var", TokenType::VAR},
        {"mut", TokenType::MUT}, {"type", TokenType::TYPE}, {"super", TokenType::SUPER},
        // Exception/Assert
        {"panic", TokenType::PANIC}, {"assert", TokenType::ASSERT},
        // Operators/Ops
        {"in", TokenType::IN}, {"is", TokenType::IS}, {"not", TokenType::NOT},
        {"and", TokenType::AND}, {"or", TokenType::OR}, {"where", TokenType::WHERE},
        {"of", TokenType::OF}, {"sizeof", TokenType::SIZEOF}, {"typeof", TokenType::TYPEOF},
        {"unsafe", TokenType::UNSAFE}, {"extern", TokenType::EXTERN},
        // Values
        {"true", TokenType::TRUE}, {"false", TokenType::FALSE}, {"nil", TokenType::NIL},
        {"self", TokenType::SELF}, {"this", TokenType::THIS}, {"new", TokenType::NEW},
        {"delete", TokenType::DELETE}, {"sys", TokenType::SYS}, {"save", TokenType::SAVE},
        {"load", TokenType::LOAD}, {"print", TokenType::PRINT},
        // Storage Types
        {"i8", TokenType::T_I8}, {"i16", TokenType::T_I16}, {"i32", TokenType::T_I32},
        {"i64", TokenType::T_I64}, {"i128", TokenType::T_I128},
        {"u8", TokenType::T_U8}, {"u16", TokenType::T_U16}, {"u32", TokenType::T_U32},
        {"u64", TokenType::T_U64}, {"u128", TokenType::T_U128},
        {"f32", TokenType::T_F32}, {"f64", TokenType::T_F64},
        {"bool", TokenType::T_BOOL}, {"char", TokenType::T_CHAR}, {"str", TokenType::T_STR},
        {"string", TokenType::T_STRING}, {"int", TokenType::T_INT}, {"float", TokenType::T_FLOAT},
        {"void", TokenType::T_VOID}, {"any", TokenType::T_ANY}, {"Type", TokenType::T_TYPE},
        {"Any", TokenType::T_ANY}, {"usize", TokenType::T_USIZE}, {"isize", TokenType::T_ISIZE}
    };
    // Load aliases from env once per process
    static bool aliasesLoaded = false;
    if (!aliasesLoaded) {
        aliasesLoaded = true;
        Config cfg = Config::fromEnv();
        for (const auto &kv : cfg.aliases) {
            const std::string &logical = kv.first;
            const std::string &alias = kv.second;
            if (logical == "print") keywords[alias] = TokenType::PRINT;
            if (logical == "let") keywords[alias] = TokenType::LET;
            if (logical == "const") keywords[alias] = TokenType::CONST;
            if (logical == "function") keywords[alias] = TokenType::FUNCTION;
            if (logical == "save") keywords[alias] = TokenType::SAVE;
            if (logical == "load") keywords[alias] = TokenType::LOAD;
            if (logical == "sys") keywords[alias] = TokenType::SYS;
            if (logical == "import") keywords[alias] = TokenType::IMPORT;
        }
    }
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        tokens.push_back({it->second, text, line_, startColumn_});
    } else {
        tokens.push_back({TokenType::IDENTIFIER, text, line_, startColumn_});
    }
}

void Lexer::number(std::vector<Token> &tokens) {
    while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        advance();
        while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
    }
    addToken(tokens, TokenType::NUMBER);
}

void Lexer::stringLiteral(std::vector<Token> &tokens) {
    // Check for raw string (r"...")
    bool isRaw = false;
    if (peek() == 'r' || peek() == 'R') {
        char next = peekNext();
        if (next == '"') {
            isRaw = true;
            advance(); // consume 'r'
        }
    }
    
    // Check for character literal (single quote)
    if (source_[start_] == '\'') {
        charLiteral(tokens);
        return;
    }
    
    while (!isAtEnd() && peek() != '"') {
        if (!isRaw && peek() == '\\') {
            advance(); // consume backslash
            if (isAtEnd()) break;
            // Handle escape sequences
            char escaped = peek();
            switch (escaped) {
                case 'n': advance(); break;
                case 't': advance(); break;
                case 'r': advance(); break;
                case '\\': advance(); break;
                case '"': advance(); break;
                default: advance(); break;
            }
        } else {
            if (peek() == '\n') { line_++; column_ = 1; }
            advance();
        }
    }
    if (isAtEnd()) {
        tokens.push_back({TokenType::INVALID, "Unterminated string", line_, column_});
        return;
    }
    advance(); // closing quote
    TokenType type = isRaw ? TokenType::RAW_STRING : TokenType::STRING;
    tokens.push_back({type, source_.substr(start_ + (isRaw ? 2 : 1), current_ - start_ - (isRaw ? 3 : 2)), line_, startColumn_});
}

void Lexer::charLiteral(std::vector<Token> &tokens) {
    advance(); // consume opening quote
    if (isAtEnd()) {
        tokens.push_back({TokenType::INVALID, "Unterminated character literal", line_, column_});
        return;
    }
    
    char value = peek();
    if (value == '\\') {
        advance(); // consume backslash
        if (isAtEnd()) {
            tokens.push_back({TokenType::INVALID, "Unterminated character literal", line_, column_});
            return;
        }
        char escaped = peek();
        switch (escaped) {
            case 'n': value = '\n'; break;
            case 't': value = '\t'; break;
            case 'r': value = '\r'; break;
            case '\\': value = '\\'; break;
            case '\'': value = '\''; break;
            default: value = escaped; break;
        }
        advance();
    } else {
        advance();
    }
    
    if (isAtEnd() || peek() != '\'') {
        tokens.push_back({TokenType::INVALID, "Unterminated character literal", line_, column_});
        return;
    }
    advance(); // closing quote
    tokens.push_back({TokenType::CHAR, std::string(1, value), line_, startColumn_});
}

void Lexer::scanToken(std::vector<Token> &tokens) {
    char c = advance();
    switch (c) {
        case '(': addToken(tokens, TokenType::LEFT_PAREN); break;
        case ')': addToken(tokens, TokenType::RIGHT_PAREN); break;
        case '{': addToken(tokens, TokenType::LEFT_BRACE); break;
        case '}': addToken(tokens, TokenType::RIGHT_BRACE); break;
        case '[': addToken(tokens, TokenType::LEFT_BRACKET); break;
        case ']': addToken(tokens, TokenType::RIGHT_BRACKET); break;
        case '+': 
            if (match('+')) addToken(tokens, TokenType::PLUS_PLUS);
            else if (match('=')) addToken(tokens, TokenType::PLUS_EQUAL);
            else addToken(tokens, TokenType::PLUS); 
            break;
        case '-': 
            if (match('-')) addToken(tokens, TokenType::MINUS_MINUS);
            else if (match('=')) addToken(tokens, TokenType::MINUS_EQUAL);
            else addToken(tokens, TokenType::MINUS); 
            break;
        case '*': 
            if (match('=')) addToken(tokens, TokenType::STAR_EQUAL);
            else addToken(tokens, TokenType::STAR); 
            break;
        case '/':
            if (peek() == '/') {
                while (!isAtEnd() && peek() != '\n') advance();
            } else if (match('=')) {
                addToken(tokens, TokenType::SLASH_EQUAL);
            } else {
                addToken(tokens, TokenType::SLASH);
            }
            break;
        case '%': addToken(tokens, TokenType::PERCENT); break;
        case '&': if (match('&')) addToken(tokens, TokenType::AND); else tokens.push_back({TokenType::INVALID, "&", line_, startColumn_}); break;
        case '|': if (match('|')) addToken(tokens, TokenType::OR); else tokens.push_back({TokenType::INVALID, "|", line_, startColumn_}); break;
        case '!': addToken(tokens, match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
        case '=': addToken(tokens, match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
        case '<': addToken(tokens, match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
        case '>': addToken(tokens, match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
        case ';': addToken(tokens, TokenType::SEMICOLON); break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case ':': addToken(tokens, TokenType::COLON); break;
        case ',': addToken(tokens, TokenType::COMMA); break;
        case '?': addToken(tokens, TokenType::QUESTION); break;
        case '.': addToken(tokens, TokenType::DOT); break;
        case '\n':
            line_++;
            column_ = 1;
            break;
        case '"': 
        case '\'': stringLiteral(tokens); break;
        case '#':
            while (!isAtEnd() && peek() != '\n') advance();
            break;
        default:
            if (std::isdigit(static_cast<unsigned char>(c))) {
                number(tokens);
            } else if (isAlpha(c)) {
                identifier(tokens);
            } else {
                tokens.push_back({TokenType::INVALID, std::string(1, c), line_, column_});
            }
    }
}

} // namespace kio
