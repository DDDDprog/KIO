/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include "axeon/lsp/types.hpp"

namespace kio::lsp {

struct FormattingOptions {
    int tabSize = 4;
    bool insertSpaces = true;
    bool trimTrailingWhitespace = true;
    bool insertFinalNewline = true;
    bool trimFinalNewlines = true;
};

class DocumentFormatter {
public:
    DocumentFormatter();
    ~DocumentFormatter();
    
    std::vector<TextEdit> format_document(const std::string& content, const FormattingOptions& options);
    
private:
    std::string format_code(const std::string& content, const FormattingOptions& options);
    std::string trim(const std::string& str);
    Position get_end_position(const std::string& content);
};

} // namespace kio::lsp
