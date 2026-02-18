/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include "axeon/token.hpp"

namespace kio {

// ANSI color codes for beautiful error display
namespace colors {
    constexpr const char* RESET = "\033[0m";
    constexpr const char* BOLD = "\033[1m";
    constexpr const char* DIM = "\033[2m";
    constexpr const char* UNDERLINE = "\033[4m";
    constexpr const char* RED = "\033[31m";
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN = "\033[36m";
    constexpr const char* WHITE = "\033[37m";
    constexpr const char* BRIGHT_RED = "\033[91m";
    constexpr const char* BRIGHT_GREEN = "\033[92m";
    constexpr const char* BRIGHT_YELLOW = "\033[93m";
    constexpr const char* BRIGHT_BLUE = "\033[94m";
    constexpr const char* BRIGHT_CYAN = "\033[96m";
}

// Error severity levels
enum class ErrorSeverity {
    ERROR,
    WARNING,
    NOTE,
    HELP
};

// Error codes for different error types
enum class ErrorCode {
    // Syntax errors (E001-E099)
    E001_UNEXPECTED_TOKEN = 1,
    E002_EXPECTED_TOKEN = 2,
    E003_UNTERMINATED_STRING = 3,
    E004_UNTERMINATED_COMMENT = 4,
    E005_INVALID_NUMBER = 5,
    E006_INVALID_ESCAPE = 6,
    
    // Declaration errors (E100-E199)
    E100_VARIABLE_DECLARATION = 100,
    E101_FUNCTION_DECLARATION = 101,
    E102_CLASS_DECLARATION = 102,
    E103_MODULE_DECLARATION = 103,
    E104_CONST_DECLARATION = 104,
    
    // Type errors (E200-E299)
    E200_TYPE_MISMATCH = 200,
    E201_UNKNOWN_TYPE = 201,
    E202_INVALID_CAST = 202,
    
    // Semantic errors (E300-E399)
    E300_UNDEFINED_VARIABLE = 300,
    E301_UNDEFINED_FUNCTION = 301,
    E302_REDEFINED_VARIABLE = 302,
    E303_REDEFINED_FUNCTION = 303,
    E304_INVALID_ASSIGNMENT = 304,
    
    // Control flow errors (E400-E499)
    E400_BREAK_OUTSIDE_LOOP = 400,
    E401_CONTINUE_OUTSIDE_LOOP = 401,
    E402_RETURN_OUTSIDE_FUNCTION = 402,
    
    // General errors (E500+)
    E500_INTERNAL_ERROR = 500,
    E999_UNKNOWN = 999
};

// Source error with full context
struct SourceError {
    int line;
    int column;
    int end_column;  // For multi-character errors
    std::string message;
    std::string source_line;
    std::string file_path;
    ErrorSeverity severity;
    ErrorCode code;
    std::string hint;  // Optional help text
    
    SourceError(int l, int c, const std::string& msg, 
                const std::string& src = "", const std::string& file = "",
                ErrorSeverity sev = ErrorSeverity::ERROR,
                ErrorCode err_code = ErrorCode::E999_UNKNOWN,
                const std::string& h = "")
        : line(l), column(c), end_column(c + 1), message(msg), source_line(src), 
          file_path(file), severity(sev), code(err_code), hint(h) {}
    
    SourceError(int l, int c, int end_c, const std::string& msg, 
                const std::string& src = "", const std::string& file = "",
                ErrorSeverity sev = ErrorSeverity::ERROR,
                ErrorCode err_code = ErrorCode::E999_UNKNOWN,
                const std::string& h = "")
        : line(l), column(c), end_column(end_c), message(msg), source_line(src), 
          file_path(file), severity(sev), code(err_code), hint(h) {}
};

class ErrorRecovery {
public:
    ErrorRecovery();
    
    // Report errors
    void reportError(const Token& token, const std::string& message);
    void reportError(int line, int column, const std::string& message);
    void reportErrorWithContext(int line, int column, const std::string& message,
                                 const std::string& source_line, const std::string& file_path = "");
    
    // Report errors with code and hint
    void reportError(const Token& token, const std::string& message, 
                     ErrorCode code, const std::string& hint = "");
    void reportError(int line, int column, const std::string& message,
                     ErrorCode code, const std::string& hint = "");
    
    // Report warnings
    void reportWarning(const Token& token, const std::string& message,
                       ErrorCode code = ErrorCode::E999_UNKNOWN, const std::string& hint = "");
    void reportWarning(int line, int column, const std::string& message,
                       ErrorCode code = ErrorCode::E999_UNKNOWN, const std::string& hint = "");
    
    // Report notes and help
    void reportNote(int line, int column, const std::string& message);
    void reportHelp(int line, int column, const std::string& message);
    
    void synchronize(const Token& token);
    
    void clear();
    bool hasErrors() const;
    bool hasWarnings() const;
    int errorCount() const;
    int warningCount() const;
    std::vector<std::string> getErrors() const;
    std::string getErrorSummary() const;
    std::string formatError(const SourceError& error) const;
    void resetPanicMode();
    
    // Set source code for context-aware errors
    void setSourceCode(const std::string& source);
    void setFilePath(const std::string& path);
    
    // Get all source errors for advanced display
    const std::vector<SourceError>& getSourceErrors() const { return source_errors_; }

private:
    std::vector<std::string> errors_;
    std::vector<SourceError> source_errors_;
    int error_count_;
    int warning_count_;
    bool panic_mode_;
    std::string source_code_;
    std::string file_path_;
    
    std::string getSourceLine(int line) const;
    std::vector<std::pair<int, std::string>> getSourceLines(int start_line, int end_line) const;
    std::string createErrorArrow(int column, int length = 1) const;
    std::string getSeverityLabel(ErrorSeverity severity) const;
    std::string getSeverityColor(ErrorSeverity severity) const;
    std::string formatErrorCode(ErrorCode code) const;
};

} // namespace kio
