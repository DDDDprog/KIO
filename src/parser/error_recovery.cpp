/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/error_recovery.hpp"
#include "kio/token.hpp"
#include <sstream>

namespace kio {

ErrorRecovery::ErrorRecovery() : error_count_(0), panic_mode_(false) {}

void ErrorRecovery::reportError(const Token& token, const std::string& message) {
    if (panic_mode_) return; // Don't report cascading errors
    
    std::ostringstream oss;
    oss << "[line " << token.line << ":" << token.column << "] Error: " << message;
    errors_.push_back(oss.str());
    error_count_++;
    panic_mode_ = true;
}

void ErrorRecovery::reportError(int line, int column, const std::string& message) {
    if (panic_mode_) return;
    
    std::ostringstream oss;
    oss << "[line " << line << ":" << column << "] Error: " << message;
    errors_.push_back(oss.str());
    error_count_++;
    panic_mode_ = true;
}

void ErrorRecovery::synchronize(const Token& token) {
    panic_mode_ = false;
    
    // Skip tokens until we find a statement boundary
    // This helps recover from errors by finding the next statement
    TokenType type = token.type;
    
    // Statement starters
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
    error_count_ = 0;
    panic_mode_ = false;
}

bool ErrorRecovery::hasErrors() const {
    return error_count_ > 0;
}

int ErrorRecovery::errorCount() const {
    return error_count_;
}

std::vector<std::string> ErrorRecovery::getErrors() const {
    return errors_;
}

std::string ErrorRecovery::getErrorSummary() const {
    if (errors_.empty()) return "No errors.";
    
    std::ostringstream oss;
    oss << "Found " << error_count_ << " error(s):\n";
    for (const auto& error : errors_) {
        oss << error << "\n";
    }
    return oss.str();
}

void ErrorRecovery::resetPanicMode() {
    panic_mode_ = false;
}

} // namespace kio
