/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/token_stream.hpp"
#include "kio/token.hpp"
#include <stdexcept>

namespace kio {

TokenStream::TokenStream(const std::vector<Token>& tokens) 
    : tokens_(tokens), current_(0) {}

bool TokenStream::hasMore() const {
    return current_ < tokens_.size();
}

const Token& TokenStream::peek() const {
    if (current_ >= tokens_.size()) {
        throw std::out_of_range("TokenStream: Attempted to peek past end");
    }
    return tokens_[current_];
}

const Token& TokenStream::peekAhead(size_t offset) const {
    size_t pos = current_ + offset;
    if (pos >= tokens_.size()) {
        throw std::out_of_range("TokenStream: Attempted to peek ahead past end");
    }
    return tokens_[pos];
}

const Token& TokenStream::advance() {
    if (current_ >= tokens_.size()) {
        throw std::out_of_range("TokenStream: Attempted to advance past end");
    }
    return tokens_[current_++];
}

const Token& TokenStream::previous() const {
    if (current_ == 0) {
        throw std::out_of_range("TokenStream: Attempted to get previous at start");
    }
    return tokens_[current_ - 1];
}

bool TokenStream::check(TokenType type) const {
    if (!hasMore()) return false;
    return peek().type == type;
}

bool TokenStream::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool TokenStream::matchAny(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (match(type)) {
            return true;
        }
    }
    return false;
}

void TokenStream::reset() {
    current_ = 0;
}

size_t TokenStream::position() const {
    return current_;
}

void TokenStream::seek(size_t pos) {
    if (pos > tokens_.size()) {
        throw std::out_of_range("TokenStream: Attempted to seek past end");
    }
    current_ = pos;
}

bool TokenStream::isAtEnd() const {
    if (current_ >= tokens_.size()) return true;
    return tokens_[current_].type == TokenType::END_OF_FILE;
}

} // namespace kio
