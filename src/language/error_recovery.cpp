/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/error_recovery.hpp"
#include "axeon/token.hpp"
#include <sstream>
#include <iomanip>

namespace kio {

ErrorRecovery::ErrorRecovery() : error_count_(0), warning_count_(0), panic_mode_(false) {}

void ErrorRecovery::setSourceCode(const std::string& source) {
    source_code_ = source;
}

void ErrorRecovery::setFilePath(const std::string& path) {
    file_path_ = path;
}

std::string ErrorRecovery::getSourceLine(int line) const {
    if (source_code_.empty()) return "";
    
    std::istringstream iss(source_code_);
    std::string result;
    int current_line = 1;
    
    while (std::getline(iss, result)) {
        if (current_line == line) {
            return result;
        }
        current_line++;
    }
    return "";
}

// Get multiple source lines for context (lines before and after)
std::vector<std::pair<int, std::string>> ErrorRecovery::getSourceLines(int start_line, int end_line) const {
    std::vector<std::pair<int, std::string>> result;
    if (source_code_.empty()) return result;
    
    std::istringstream iss(source_code_);
    std::string line;
    int current_line = 1;
    
    while (std::getline(iss, line)) {
        if (current_line >= start_line && current_line <= end_line) {
            result.push_back({current_line, line});
        }
        if (current_line > end_line) break;
        current_line++;
    }
    return result;
}

std::string ErrorRecovery::createErrorArrow(int column, int length) const {
    std::ostringstream oss;
    // Create spaces to align with the error position
    for (int i = 0; i < column - 1; ++i) {
        oss << " ";
    }
    // Add arrow characters
    oss << colors::BRIGHT_RED << "^";
    for (int i = 1; i < length; ++i) {
        oss << "~";
    }
    oss << colors::RESET;
    return oss.str();
}

std::string ErrorRecovery::getSeverityLabel(ErrorSeverity severity) const {
    switch (severity) {
        case ErrorSeverity::ERROR: return "error";
        case ErrorSeverity::WARNING: return "warning";
        case ErrorSeverity::NOTE: return "note";
        case ErrorSeverity::HELP: return "help";
        default: return "error";
    }
}

std::string ErrorRecovery::getSeverityColor(ErrorSeverity severity) const {
    switch (severity) {
        case ErrorSeverity::ERROR: return colors::BRIGHT_RED;
        case ErrorSeverity::WARNING: return colors::BRIGHT_YELLOW;
        case ErrorSeverity::NOTE: return colors::BRIGHT_CYAN;
        case ErrorSeverity::HELP: return colors::BRIGHT_GREEN;
        default: return colors::BRIGHT_RED;
    }
}

std::string ErrorRecovery::formatErrorCode(ErrorCode code) const {
    return "E" + std::string(3 - std::to_string(static_cast<int>(code)).length(), '0') + 
           std::to_string(static_cast<int>(code));
}

std::string ErrorRecovery::formatError(const SourceError& error) const {
    std::ostringstream oss;
    
    // Error header with severity, code, and message
    oss << "\n";
    oss << colors::BOLD << getSeverityColor(error.severity) 
        << getSeverityLabel(error.severity) << colors::RESET 
        << colors::BOLD << colors::DIM << "[" << formatErrorCode(error.code) << "]" 
        << colors::RESET << colors::BOLD << ": " << error.message << colors::RESET << "\n";
    
    // File location
    oss << colors::DIM << "  --> " << colors::RESET;
    if (!error.file_path.empty()) {
        oss << colors::CYAN << error.file_path << colors::RESET;
    } else {
        oss << colors::CYAN << "<stdin>" << colors::RESET;
    }
    oss << ":" << error.line << ":" << error.column << "\n";
    
    // Show source code with context lines (2 before and 2 after)
    constexpr int CONTEXT_LINES = 2;
    int start_line = std::max(1, error.line - CONTEXT_LINES);
    int end_line = error.line + CONTEXT_LINES;
    
    auto lines = getSourceLines(start_line, end_line);
    
    if (!lines.empty()) {
        oss << colors::DIM << "   |" << colors::RESET << "\n";
        
        for (const auto& [line_num, line_content] : lines) {
            // Highlight the error line
            if (line_num == error.line) {
                // Line number and source code
                oss << colors::DIM << std::setw(3) << line_num << " |" << colors::RESET 
                    << " " << line_content << "\n";
                
                // Error pointer
                oss << colors::DIM << "   |" << colors::RESET << " " 
                    << createErrorArrow(error.column, error.end_column - error.column + 1) << "\n";
            } else {
                // Context line (dimmed)
                oss << colors::DIM << std::setw(3) << line_num << " |" << colors::RESET 
                    << " " << colors::DIM << line_content << colors::RESET << "\n";
            }
        }
    } else if (!error.source_line.empty()) {
        oss << colors::DIM << "   |" << colors::RESET << "\n";
        
        // Line number and source code
        oss << colors::DIM << std::setw(3) << error.line << " |" << colors::RESET 
            << " " << error.source_line << "\n";
        
        // Error pointer
        oss << colors::DIM << "   |" << colors::RESET << " " 
            << createErrorArrow(error.column, error.end_column - error.column + 1) << "\n";
    } else {
        oss << colors::DIM << "   |" << colors::RESET << "\n";
        oss << colors::DIM << std::setw(3) << error.line << " |" << colors::RESET << "\n";
        oss << colors::DIM << "   |" << colors::RESET << " " 
            << createErrorArrow(error.column, error.end_column - error.column + 1) << "\n";
    }
    
    // Show hint if available
    if (!error.hint.empty()) {
        oss << colors::DIM << "   |" << colors::RESET << "\n";
        oss << colors::DIM << "   = " << colors::RESET 
            << colors::BOLD << colors::BRIGHT_GREEN << "help: " << colors::RESET
            << colors::GREEN << error.hint << colors::RESET << "\n";
    }
    
    oss << colors::DIM << "   |" << colors::RESET << "\n";
    
    return oss.str();
}

void ErrorRecovery::reportError(const Token& token, const std::string& message) {
    reportError(token, message, ErrorCode::E999_UNKNOWN, "");
}

void ErrorRecovery::reportError(const Token& token, const std::string& message, 
                                 ErrorCode code, const std::string& hint) {
    if (panic_mode_) return;
    
    std::string source_line = getSourceLine(token.line);
    source_errors_.emplace_back(token.line, token.column, message, source_line, file_path_,
                                 ErrorSeverity::ERROR, code, hint);
    
    // Also keep legacy format for compatibility
    std::ostringstream oss;
    oss << "[line " << token.line << ":" << token.column << "] Error: " << message;
    errors_.push_back(oss.str());
    error_count_++;
    panic_mode_ = true;
}

void ErrorRecovery::reportError(int line, int column, const std::string& message) {
    reportError(line, column, message, ErrorCode::E999_UNKNOWN, "");
}

void ErrorRecovery::reportError(int line, int column, const std::string& message,
                                 ErrorCode code, const std::string& hint) {
    if (panic_mode_) return;
    
    std::string source_line = getSourceLine(line);
    source_errors_.emplace_back(line, column, message, source_line, file_path_,
                                 ErrorSeverity::ERROR, code, hint);
    
    // Also keep legacy format for compatibility
    std::ostringstream oss;
    oss << "[line " << line << ":" << column << "] Error: " << message;
    errors_.push_back(oss.str());
    error_count_++;
    panic_mode_ = true;
}

void ErrorRecovery::reportErrorWithContext(int line, int column, const std::string& message,
                                            const std::string& source_line, const std::string& file_path) {
    if (panic_mode_) return;
    
    source_errors_.emplace_back(line, column, message, source_line, file_path);
}

void ErrorRecovery::reportWarning(const Token& token, const std::string& message,
                                   ErrorCode code, const std::string& hint) {
    std::string source_line = getSourceLine(token.line);
    source_errors_.emplace_back(token.line, token.column, message, source_line, file_path_,
                                 ErrorSeverity::WARNING, code, hint);
    warning_count_++;
}

void ErrorRecovery::reportWarning(int line, int column, const std::string& message,
                                   ErrorCode code, const std::string& hint) {
    std::string source_line = getSourceLine(line);
    source_errors_.emplace_back(line, column, message, source_line, file_path_,
                                 ErrorSeverity::WARNING, code, hint);
    warning_count_++;
}

void ErrorRecovery::reportNote(int line, int column, const std::string& message) {
    std::string source_line = getSourceLine(line);
    source_errors_.emplace_back(line, column, message, source_line, file_path_,
                                 ErrorSeverity::NOTE, ErrorCode::E999_UNKNOWN, "");
}

void ErrorRecovery::reportHelp(int line, int column, const std::string& message) {
    std::string source_line = getSourceLine(line);
    source_errors_.emplace_back(line, column, message, source_line, file_path_,
                                 ErrorSeverity::HELP, ErrorCode::E999_UNKNOWN, "");
}

void ErrorRecovery::synchronize(const Token& token) {
    panic_mode_ = false;
    
    TokenType type = token.type;
    
    if (type == TokenType::SEMICOLON || 
        type == TokenType::PRINT ||
        type == TokenType::LET ||
        type == TokenType::CONST ||
        type == TokenType::FUNCTION ||
        type == TokenType::IF ||
        type == TokenType::WHILE ||
        type == TokenType::FOR ||
        type == TokenType::RETURN ||
        type == TokenType::CLASS ||
        type == TokenType::NAMESPACE ||
        type == TokenType::END_OF_FILE) {
        return;
    }
}

void ErrorRecovery::clear() {
    errors_.clear();
    source_errors_.clear();
    error_count_ = 0;
    warning_count_ = 0;
    panic_mode_ = false;
}

bool ErrorRecovery::hasErrors() const {
    return error_count_ > 0;
}

bool ErrorRecovery::hasWarnings() const {
    return warning_count_ > 0;
}

int ErrorRecovery::errorCount() const {
    return error_count_;
}

int ErrorRecovery::warningCount() const {
    return warning_count_;
}

std::vector<std::string> ErrorRecovery::getErrors() const {
    return errors_;
}

std::string ErrorRecovery::getErrorSummary() const {
    if (source_errors_.empty() && errors_.empty()) {
        return std::string(colors::GREEN) + "✓ No errors." + colors::RESET + "\n";
    }
    
    std::ostringstream oss;
    
    // Print beautiful formatted errors if available
    if (!source_errors_.empty()) {
        for (const auto& error : source_errors_) {
            oss << formatError(error);
        }
        
        // Summary line
        oss << "\n";
        if (error_count_ > 0) {
            oss << colors::BOLD << colors::BRIGHT_RED << "✗ " 
                << error_count_ << " error(s)" << colors::RESET;
        }
        if (warning_count_ > 0) {
            if (error_count_ > 0) oss << ", ";
            oss << colors::BOLD << colors::BRIGHT_YELLOW << "⚠ " 
                << warning_count_ << " warning(s)" << colors::RESET;
        }
        oss << "\n";
    } else {
        // Fallback to legacy format
        oss << colors::BOLD << colors::BRIGHT_RED << "Found " << error_count_ 
            << " error(s):" << colors::RESET << "\n\n";
        for (const auto& error : errors_) {
            oss << colors::BRIGHT_YELLOW << "  ⚠ " << error << colors::RESET << "\n";
        }
    }
    return oss.str();
}

void ErrorRecovery::resetPanicMode() {
    panic_mode_ = false;
}

} // namespace kio
