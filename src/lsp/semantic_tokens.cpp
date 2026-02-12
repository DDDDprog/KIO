/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/lsp/semantic_tokens.hpp"
#include "kio/lexer.hpp"
#include <algorithm>

namespace kio::lsp {

SemanticTokensProvider::SemanticTokensProvider() {
    initialize_token_types();
}

SemanticTokensProvider::~SemanticTokensProvider() = default;

std::vector<uint32_t> SemanticTokensProvider::get_semantic_tokens(const std::string& content) {
    std::vector<SemanticToken> tokens = analyze_tokens(content);
    return encode_tokens(tokens);
}

std::vector<SemanticToken> SemanticTokensProvider::analyze_tokens(const std::string& content) {
    std::vector<SemanticToken> semantic_tokens;
    
    try {
        kio::Lexer lexer(content);
        auto tokens = lexer.tokenize();
        
        for (const auto& token : tokens) {
            SemanticToken semantic_token;
            semantic_token.line = token.line - 1; // LSP is 0-indexed
            semantic_token.character = token.column - 1;
            semantic_token.length = static_cast<int>(token.lexeme.length());
            semantic_token.type = map_token_type(token.type);
            semantic_token.modifiers = 0;
            
            semantic_tokens.push_back(semantic_token);
        }
    } catch (const std::exception&) {
        // If tokenization fails, return empty tokens
    }
    
    return semantic_tokens;
}

uint32_t SemanticTokensProvider::map_token_type(kio::TokenType token_type) {
    switch (token_type) {
        case kio::TokenType::PRINT:
        case kio::TokenType::LET:
        case kio::TokenType::SAVE:
        case kio::TokenType::LOAD:
        case kio::TokenType::SYS:
        case kio::TokenType::IMPORT:
        case kio::TokenType::IF:
        case kio::TokenType::ELSE:
        case kio::TokenType::WHILE:
            return static_cast<uint32_t>(SemanticTokenType::Keyword);
        case kio::TokenType::IDENTIFIER:
            return static_cast<uint32_t>(SemanticTokenType::Variable);
        case kio::TokenType::NUMBER:
            return static_cast<uint32_t>(SemanticTokenType::Number);
        case kio::TokenType::STRING:
            return static_cast<uint32_t>(SemanticTokenType::String);
        case kio::TokenType::PLUS:
        case kio::TokenType::MINUS:
        case kio::TokenType::STAR:
        case kio::TokenType::SLASH:
        case kio::TokenType::EQUAL:
        case kio::TokenType::EQUAL_EQUAL:
            return static_cast<uint32_t>(SemanticTokenType::Operator);
        default:
            return static_cast<uint32_t>(SemanticTokenType::Variable);
    }
}

std::vector<uint32_t> SemanticTokensProvider::encode_tokens(const std::vector<SemanticToken>& tokens) {
    std::vector<uint32_t> encoded;
    
    uint32_t prev_line = 0;
    uint32_t prev_char = 0;
    
    for (const auto& token : tokens) {
        // Delta line
        uint32_t delta_line = token.line - prev_line;
        encoded.push_back(delta_line);
        
        // Delta character (reset to 0 if on new line)
        uint32_t delta_char = (delta_line == 0) ? token.character - prev_char : token.character;
        encoded.push_back(delta_char);
        
        // Length
        encoded.push_back(token.length);
        
        // Token type
        encoded.push_back(token.type);
        
        // Token modifiers
        encoded.push_back(token.modifiers);
        
        prev_line = token.line;
        prev_char = token.character;
    }
    
    return encoded;
}

void SemanticTokensProvider::initialize_token_types() {
    // Token types are defined in the enum and don't need initialization
    // This method is kept for future extensibility
}

} // namespace kio::lsp
