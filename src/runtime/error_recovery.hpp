/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include "axeon/token.hpp"

namespace kio {

class ErrorRecovery {
public:
    ErrorRecovery();
    
    void reportError(const Token& token, const std::string& message);
    void reportError(int line, int column, const std::string& message);
    void synchronize(const Token& token);
    
    void clear();
    bool hasErrors() const;
    int errorCount() const;
    std::vector<std::string> getErrors() const;
    std::string getErrorSummary() const;
    void resetPanicMode();

private:
    std::vector<std::string> errors_;
    int error_count_;
    bool panic_mode_;
};

} // namespace kio
