/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace kio {

class Preprocessor {
public:
    Preprocessor();
    
    std::string process(const std::string& source);
    std::string processLine(const std::string& line, int lineNumber);
    
    void clear();

private:
    std::unordered_map<std::string, std::string> macros_;
    std::vector<bool> conditionals_;
    
    std::string handleInclude(const std::string& line, int lineNumber);
    void handleDefine(const std::string& line, int lineNumber);
    void handleConditional(const std::string& line, int lineNumber);
    std::string loadFile(const std::string& filename);
    std::string trim(const std::string& str);
};

} // namespace kio
