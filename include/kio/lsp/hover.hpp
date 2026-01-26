/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include "kio/lsp/types.hpp"

namespace kio::lsp {

class HoverProvider {
public:
    HoverProvider();
    ~HoverProvider();
    
    std::optional<Hover> get_hover(const std::string& content, const Position& position);
    
private:
    std::optional<WordRange> get_word_at_position(const std::string& content, const Position& position);
    std::optional<std::string> get_hover_info(const std::string& word);
    Range position_range_to_lsp_range(const std::string& content, const WordRange& word_range);
};

} // namespace kio::lsp
