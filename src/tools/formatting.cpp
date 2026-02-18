/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/lsp/formatting.hpp"
#include <sstream>
#include <algorithm>

namespace kio::lsp {

DocumentFormatter::DocumentFormatter() = default;
DocumentFormatter::~DocumentFormatter() = default;

std::vector<TextEdit> DocumentFormatter::format_document(const std::string& content, const FormattingOptions& options) {
    std::string formatted = format_code(content, options);
    
    std::vector<TextEdit> edits;
    TextEdit edit;
    edit.range.start = {0, 0};
    edit.range.end = get_end_position(content);
    edit.newText = formatted;
    edits.push_back(edit);
    
    return edits;
}

std::string DocumentFormatter::format_code(const std::string& content, const FormattingOptions& options) {
    std::stringstream ss(content);
    std::string line;
    std::string result;
    int indentLevel = 0;
    
    while (std::getline(ss, line)) {
        line = trim(line);
        if (line.empty()) {
            result += "\n";
            continue;
        }
        
        if (line.back() == '}' || line.front() == '}') indentLevel = std::max(0, indentLevel - 1);
        
        for (int i = 0; i < indentLevel * options.tabSize; ++i) result += options.insertSpaces ? " " : "\t";
        result += line + "\n";
        
        if (line.back() == '{' || line.front() == '{') indentLevel++;
    }
    
    return result;
}

std::string DocumentFormatter::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

Position DocumentFormatter::get_end_position(const std::string& content) {
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
