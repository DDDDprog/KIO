#include "kio/lexer.hpp"
#include <cctype>
#include <unordered_map>
#include "kio/config.hpp"

namespace kio {

static bool isAlpha(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
static bool isAlphaNumeric(char c) { return isAlpha(c) || std::isdigit(static_cast<unsigned char>(c)); }

Lexer::Lexer(const std::string &source) : source_(source) {}

std::vector<Token> Lexer::scanTokens() {
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        start_ = current_;
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
    tokens.push_back({type, source_.substr(start_, current_ - start_), line_, column_});
}

void Lexer::identifier(std::vector<Token> &tokens) {
    while (isAlphaNumeric(peek())) advance();
    std::string text = source_.substr(start_, current_ - start_);
    static std::unordered_map<std::string, TokenType> keywords{
        {"print", TokenType::PRINT},
        {"let", TokenType::LET},
        {"save", TokenType::SAVE},
        {"load", TokenType::LOAD},
        {"sys", TokenType::SYS},
        {"import", TokenType::IMPORT},
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
            if (logical == "save") keywords[alias] = TokenType::SAVE;
            if (logical == "load") keywords[alias] = TokenType::LOAD;
            if (logical == "sys") keywords[alias] = TokenType::SYS;
            if (logical == "import") keywords[alias] = TokenType::IMPORT;
        }
    }
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        tokens.push_back({it->second, text, line_, column_});
    } else {
        tokens.push_back({TokenType::IDENTIFIER, text, line_, column_});
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
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') { line_++; column_ = 1; }
        advance();
    }
    if (isAtEnd()) {
        tokens.push_back({TokenType::INVALID, "Unterminated string", line_, column_});
        return;
    }
    advance(); // closing quote
    tokens.push_back({TokenType::STRING, source_.substr(start_ + 1, current_ - start_ - 2), line_, column_});
}

void Lexer::scanToken(std::vector<Token> &tokens) {
    char c = advance();
    switch (c) {
        case '(': addToken(tokens, TokenType::LEFT_PAREN); break;
        case ')': addToken(tokens, TokenType::RIGHT_PAREN); break;
        case '+': addToken(tokens, TokenType::PLUS); break;
        case '-': addToken(tokens, TokenType::MINUS); break;
        case '*': addToken(tokens, TokenType::STAR); break;
        case '/':
            if (peek() == '/') {
                while (!isAtEnd() && peek() != '\n') advance();
            } else {
                addToken(tokens, TokenType::SLASH);
            }
            break;
        case '=': addToken(tokens, TokenType::EQUAL); break;
        case ';': addToken(tokens, TokenType::SEMICOLON); break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            line_++;
            column_ = 1;
            break;
        case '"': stringLiteral(tokens); break;
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
