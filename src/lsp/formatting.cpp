/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/lsp/formatting.hpp"
#include <sstream>
#include <regex>

namespace kio::lsp {

FormattingProvider::FormattingProvider() = default;
FormattingProvider::~FormattingProvider() = default;

std::vector<TextEdit> FormattingProvider::format_document(const std::string& content) {
    std::string formatted = format_code(content);
    
    if (formatted == content) {
        return {}; // No changes needed
    }
    
    // Return a single edit that replaces the entire document
    TextEdit edit;
    edit.range = {{0, 0}, get_end_position(content)};
    edit.new_text = formatted;
    
    return {edit};
}

std::string FormattingProvider::format_code(const std::string& content) {
    std::istringstream input(content);
    std::ostringstream output;
    std::string line;
    int indent_level = 0;
    
    while (std::getline(input, line)) {
        // Trim whitespace
        line = trim(line);
        
        if (line.empty()) {
            output << "\n";
            continue;
        }
        
        // Adjust indent level for closing braces
        if (line.find('}') == 0) {
            indent_level = std::max(0, indent_level - 1);
        }
        
        // Apply indentation
        for (int i = 0; i < indent_level; ++i) {
            output << "    "; // 4 spaces per indent level
        }
        
        output << line << "\n";
        
        // Adjust indent level for opening braces
        if (line.back() == '{') {
            indent_level++;
        }
    }
    
    return output.str();
}

std::string FormattingProvider::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

Position FormattingProvider::get_end_position(const std::string& content) {
    Position pos{0, 0};
    for (char c : content) {
        if (c == '\n') {
            pos.line++;
            pos.character = 0;
        } else {
            pos.character++;
        }
    }
    return pos;
}

} // namespace kio::lsp
