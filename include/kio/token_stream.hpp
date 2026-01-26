/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <vector>
#include "kio/token.hpp"

namespace kio {

class TokenStream {
public:
    explicit TokenStream(const std::vector<Token>& tokens);
    
    bool hasMore() const;
    const Token& peek() const;
    const Token& peekAhead(size_t offset) const;
    const Token& advance();
    const Token& previous() const;
    
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool matchAny(std::initializer_list<TokenType> types);
    
    void reset();
    size_t position() const;
    void seek(size_t pos);
    bool isAtEnd() const;

private:
    const std::vector<Token>& tokens_;
    size_t current_;
};

} // namespace kio
