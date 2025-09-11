/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace kio {
namespace lsp {

enum class TokenType {
    Namespace,
    Type,
    Class,
    Enum,
    Interface,
    Struct,
    TypeParameter,
    Parameter,
    Variable,
    Property,
    EnumMember,
    Event,
    Function,
    Method,
    Macro,
    Keyword,
    Modifier,
    Comment,
    String,
    Number,
    Regexp,
    Operator
};

enum class TokenModifier {
    Declaration,
    Definition,
    Readonly,
    Static,
    Deprecated,
    Abstract,
    Async,
    Modification,
    Documentation,
    DefaultLibrary
};

struct SemanticToken {
    int line;
    int startChar;
    int length;
    TokenType tokenType;
    std::vector<TokenModifier> modifiers;
    
    SemanticToken(int l = 0, int s = 0, int len = 0, TokenType type = TokenType::Variable)
        : line(l), startChar(s), length(len), tokenType(type) {}
};

class SemanticTokensProvider {
public:
    SemanticTokensProvider();
    ~SemanticTokensProvider();
    
    std::vector<int> getSemanticTokens(const std::string& content);
    std::vector<SemanticToken> getSemanticTokensStructured(const std::string& content);
    
    static std::vector<std::string> getTokenTypes();
    static std::vector<std::string> getTokenModifiers();

private:
    std::vector<SemanticToken> analyzeTokens(const std::string& content);
    TokenType classifyToken(const std::string& token, const std::string& context);
    std::vector<int> encodeTokens(const std::vector<SemanticToken>& tokens);
    
    std::unordered_map<std::string, TokenType> keyword_map_;
    void initializeKeywordMap();
};

} // namespace lsp
} // namespace kio
