/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/lsp/diagnostics.hpp"
#include "axeon/lexer.hpp"
#include "axeon/parser.hpp"
#include <sstream>

namespace kio::lsp {

DiagnosticsProvider::DiagnosticsProvider() = default;
DiagnosticsProvider::~DiagnosticsProvider() = default;

std::vector<Diagnostic> DiagnosticsProvider::analyze_document(const std::string& content) {
    std::vector<Diagnostic> diagnostics;
    
    try {
        kio::Lexer lexer(content);
        auto tokens = lexer.tokenize();
        kio::Parser parser(tokens);
        auto stmts = parser.parse();
        
        analyze_semantics(stmts, diagnostics);
        
    } catch (const std::exception& e) {
        Diagnostic diag;
        diag.range = {{0, 0}, {0, 100}};
        diag.severity = DiagnosticSeverity::Error;
        diag.message = e.what();
        diag.source = "kio-lsp";
        diagnostics.push_back(diag);
    }
    
    return diagnostics;
}

void DiagnosticsProvider::analyze_semantics(const std::vector<StmtPtr>& stmts, 
                                          std::vector<Diagnostic>& diagnostics) {
    for (const auto& stmt : stmts) {
        check_statement(stmt, diagnostics);
    }
}

void DiagnosticsProvider::check_statement(const StmtPtr& stmt, std::vector<Diagnostic>& diagnostics) {
    if (!stmt) return;
    check_undefined_variables(stmt, diagnostics);
    check_type_mismatches(stmt, diagnostics);
    check_unreachable_code(stmt, diagnostics);
    
    // For blocks, recurse
    if (auto block = std::get_if<Stmt::Block>(&stmt->node)) {
        analyze_semantics(block->statements, diagnostics);
    } else if (auto ifStmt = std::get_if<Stmt::If>(&stmt->node)) {
        check_statement(ifStmt->thenBranch, diagnostics);
        if (ifStmt->elseBranch) check_statement(ifStmt->elseBranch, diagnostics);
    } else if (auto whileStmt = std::get_if<Stmt::While>(&stmt->node)) {
        check_statement(whileStmt->body, diagnostics);
    }
}

void DiagnosticsProvider::check_undefined_variables(const StmtPtr& stmt,
                                                   std::vector<Diagnostic>& diagnostics) {
    // Placeholder
}

void DiagnosticsProvider::check_type_mismatches(const StmtPtr& stmt,
                                               std::vector<Diagnostic>& diagnostics) {
    // Placeholder
}

void DiagnosticsProvider::check_unreachable_code(const StmtPtr& stmt,
                                                std::vector<Diagnostic>& diagnostics) {
    // Placeholder
}

} // namespace kio::lsp
