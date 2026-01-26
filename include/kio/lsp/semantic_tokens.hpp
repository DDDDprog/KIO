/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "kio/lsp/types.hpp"
#include "kio/token.hpp"

namespace kio::lsp {

class SemanticTokensProvider {
public:
    SemanticTokensProvider();
    ~SemanticTokensProvider();
    
    std::vector<uint32_t> get_semantic_tokens(const std::string& content);
    
private:
    std::vector<SemanticToken> analyze_tokens(const std::string& content);
    uint32_t map_token_type(kio::TokenType token_type);
    std::vector<uint32_t> encode_tokens(const std::vector<SemanticToken>& tokens);
    void initialize_token_types();
};

} // namespace kio::lsp
