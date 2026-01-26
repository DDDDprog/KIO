/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>
#include "kio/lsp/types.hpp"
#include "kio/ast.hpp"
#include "kio/lexer.hpp"
#include "kio/parser.hpp"

namespace kio::lsp {

struct Document {
    std::string content;
    int version;
    bool is_dirty;
    
    Document() : version(1), is_dirty(false) {}
};

class DocumentManager {
public:
    DocumentManager();
    ~DocumentManager();
    
    void open_document(const std::string& uri, const std::string& content);
    void update_document(const std::string& uri, const std::string& content, int version);
    void close_document(const std::string& uri);
    
    std::optional<Document> get_document(const std::string& uri) const;
    std::vector<std::string> get_all_uris() const;
    
    static Position offset_to_position(const std::string& content, size_t offset);
    static size_t position_to_offset(const std::string& content, const Position& pos);
    
private:
    std::unordered_map<std::string, Document> documents_;
};

} // namespace kio::lsp
