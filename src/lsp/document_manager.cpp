/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/lsp/document_manager.hpp"
#include <fstream>
#include <sstream>
#include <optional>

namespace kio::lsp {

DocumentManager::DocumentManager() = default;
DocumentManager::~DocumentManager() = default;

void DocumentManager::open_document(const std::string& uri, const std::string& content) {
    documents_[uri] = {
        .content = content,
        .version = 1,
        .is_dirty = false
    };
}

void DocumentManager::close_document(const std::string& uri) {
    documents_.erase(uri);
}

void DocumentManager::update_document(const std::string& uri, const std::string& content, int version) {
    if (auto it = documents_.find(uri); it != documents_.end()) {
        it->second.content = content;
        it->second.version = version;
        it->second.is_dirty = true;
    }
}

std::optional<Document> DocumentManager::get_document(const std::string& uri) const {
    if (auto it = documents_.find(uri); it != documents_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> DocumentManager::get_all_uris() const {
    std::vector<std::string> uris;
    uris.reserve(documents_.size());
    for (const auto& [uri, _] : documents_) {
        uris.push_back(uri);
    }
    return uris;
}

Position DocumentManager::offset_to_position(const std::string& content, size_t offset) {
    Position pos{0, 0};
    for (size_t i = 0; i < offset && i < content.size(); ++i) {
        if (content[i] == '\n') {
            pos.line++;
            pos.character = 0;
        } else {
            pos.character++;
        }
    }
    return pos;
}

size_t DocumentManager::position_to_offset(const std::string& content, const Position& pos) {
    size_t offset = 0;
    int current_line = 0;
    int current_char = 0;
    
    for (size_t i = 0; i < content.size(); ++i) {
        if (current_line == pos.line && current_char == pos.character) {
            return offset;
        }
        
        if (content[i] == '\n') {
            current_line++;
            current_char = 0;
        } else {
            current_char++;
        }
        offset++;
    }
    
    return offset;
}

} // namespace kio::lsp
