/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <vector>
#include <string>
#include <memory>
#include "axeon/lsp/types.hpp"
#include "axeon/ast.hpp"

namespace kio::lsp {

class DiagnosticsProvider {
public:
    DiagnosticsProvider();
    ~DiagnosticsProvider();
    
    std::vector<Diagnostic> analyze_document(const std::string& content);
    
private:
    void analyze_semantics(const std::vector<StmtPtr>& statements, std::vector<Diagnostic>& diagnostics);
    void check_statement(const StmtPtr& stmt, std::vector<Diagnostic>& diagnostics);
    void check_undefined_variables(const StmtPtr& stmt, std::vector<Diagnostic>& diagnostics);
    void check_type_mismatches(const StmtPtr& stmt, std::vector<Diagnostic>& diagnostics);
    void check_unreachable_code(const StmtPtr& stmt, std::vector<Diagnostic>& diagnostics);
};

} // namespace kio::lsp
