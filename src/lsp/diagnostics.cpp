#include "kio/lsp/diagnostics.hpp"
#include "kio/lexer.hpp"
#include "kio/parser.hpp"
#include <sstream>

namespace kio::lsp {

DiagnosticsProvider::DiagnosticsProvider() = default;
DiagnosticsProvider::~DiagnosticsProvider() = default;

std::vector<Diagnostic> DiagnosticsProvider::analyze_document(const std::string& content) {
    std::vector<Diagnostic> diagnostics;
    
    try {
        // Lexical analysis
        kio::Lexer lexer(content);
        auto tokens = lexer.tokenize();
        
        // Syntax analysis
        kio::Parser parser(tokens);
        auto ast = parser.parse();
        
        // Semantic analysis
        analyze_semantics(ast, diagnostics);
        
    } catch (const std::exception& e) {
        // Add parse error as diagnostic
        Diagnostic diag;
        diag.range = {{0, 0}, {0, static_cast<int>(content.find('\n'))}};
        diag.severity = DiagnosticSeverity::Error;
        diag.message = e.what();
        diag.source = "kio-lsp";
        diagnostics.push_back(diag);
    }
    
    return diagnostics;
}

void DiagnosticsProvider::analyze_semantics(const std::shared_ptr<kio::ASTNode>& node, 
                                          std::vector<Diagnostic>& diagnostics) {
    if (!node) return;
    
    // Check for undefined variables
    check_undefined_variables(node, diagnostics);
    
    // Check for type mismatches
    check_type_mismatches(node, diagnostics);
    
    // Check for unreachable code
    check_unreachable_code(node, diagnostics);
    
    // Recursively analyze child nodes
    for (const auto& child : node->children) {
        analyze_semantics(child, diagnostics);
    }
}

void DiagnosticsProvider::check_undefined_variables(const std::shared_ptr<kio::ASTNode>& node,
                                                   std::vector<Diagnostic>& diagnostics) {
    // Implementation for undefined variable checking
    // This would integrate with the symbol table from the interpreter
}

void DiagnosticsProvider::check_type_mismatches(const std::shared_ptr<kio::ASTNode>& node,
                                               std::vector<Diagnostic>& diagnostics) {
    // Implementation for type checking
    // This would use the type system to verify type compatibility
}

void DiagnosticsProvider::check_unreachable_code(const std::shared_ptr<kio::ASTNode>& node,
                                                std::vector<Diagnostic>& diagnostics) {
    // Implementation for unreachable code detection
    // This would analyze control flow to find dead code
}

} // namespace kio::lsp
