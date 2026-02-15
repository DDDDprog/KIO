/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/preprocessor.hpp"
#include <sstream>
#include <fstream>
#include <algorithm>

namespace kio {

Preprocessor::Preprocessor() {}

std::string Preprocessor::process(const std::string& source) {
    std::stringstream result;
    std::istringstream input(source);
    std::string line;
    int lineNumber = 1;
    
    while (std::getline(input, line)) {
        std::string processed = processLine(line, lineNumber);
        if (!processed.empty()) {
            result << processed << "\n";
        }
        lineNumber++;
    }
    
    return result.str();
}

std::string Preprocessor::processLine(const std::string& line, int lineNumber) {
    std::string trimmed = trim(line);
    
    // Handle #include directives
    if (trimmed.find("#include") == 0) {
        return handleInclude(trimmed, lineNumber);
    }
    
    // Handle #define directives
    if (trimmed.find("#define") == 0) {
        handleDefine(trimmed, lineNumber);
        return ""; // Don't include the define line itself
    }
    
    // Handle #ifdef / #ifndef
    if (trimmed.find("#ifdef") == 0 || trimmed.find("#ifndef") == 0) {
        handleConditional(trimmed, lineNumber);
        return ""; // Don't include conditional lines
    }
    
    // Replace macros
    std::string result = line;
    for (const auto& [macro, value] : macros_) {
        size_t pos = 0;
        while ((pos = result.find(macro, pos)) != std::string::npos) {
            result.replace(pos, macro.length(), value);
            pos += value.length();
        }
    }
    
    return result;
}

std::string Preprocessor::handleInclude(const std::string& line, int lineNumber) {
    size_t start = line.find('"');
    if (start == std::string::npos) {
        start = line.find('<');
        if (start == std::string::npos) {
            return ""; // Invalid include
        }
        size_t end = line.find('>', start + 1);
        if (end == std::string::npos) return "";
        
        std::string filename = line.substr(start + 1, end - start - 1);
        return loadFile(filename);
    } else {
        size_t end = line.find('"', start + 1);
        if (end == std::string::npos) return "";
        
        std::string filename = line.substr(start + 1, end - start - 1);
        return loadFile(filename);
    }
}

void Preprocessor::handleDefine(const std::string& line, int lineNumber) {
    size_t definePos = line.find("#define");
    if (definePos == std::string::npos) return;
    
    size_t start = definePos + 7; // Skip "#define"
    while (start < line.length() && std::isspace(line[start])) start++;
    
    size_t end = start;
    while (end < line.length() && !std::isspace(line[end])) end++;
    
    std::string macro = line.substr(start, end - start);
    
    start = end;
    while (start < line.length() && std::isspace(line[start])) start++;
    
    std::string value = line.substr(start);
    value = trim(value);
    
    macros_[macro] = value;
}

void Preprocessor::handleConditional(const std::string& line, int lineNumber) {
    // Simple conditional handling - can be expanded
    if (line.find("#ifdef") == 0) {
        size_t start = 6;
        while (start < line.length() && std::isspace(line[start])) start++;
        size_t end = start;
        while (end < line.length() && !std::isspace(line[end])) end++;
        std::string macro = line.substr(start, end - start);
        conditionals_.push_back(macros_.find(macro) != macros_.end());
    } else if (line.find("#ifndef") == 0) {
        size_t start = 7;
        while (start < line.length() && std::isspace(line[start])) start++;
        size_t end = start;
        while (end < line.length() && !std::isspace(line[end])) end++;
        std::string macro = line.substr(start, end - start);
        conditionals_.push_back(macros_.find(macro) == macros_.end());
    }
}

std::string Preprocessor::loadFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return ""; // File not found
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string Preprocessor::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

void Preprocessor::clear() {
    macros_.clear();
    conditionals_.clear();
}

} // namespace kio
