/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include <optional>
#include "axeon/lsp/types.hpp"

namespace kio::lsp {

class GotoDefinitionProvider {
public:
    GotoDefinitionProvider();
    ~GotoDefinitionProvider();
    
    std::optional<Location> get_definition(const std::string& content, const Position& position);
    
private:
    std::optional<std::string> get_symbol_at_position(const std::string& content, const Position& position);
    std::optional<Location> find_symbol_definition(const std::string& symbol, const std::string& content);
    Position offset_to_position(const std::string& content, size_t offset);
};

} // namespace kio::lsp
