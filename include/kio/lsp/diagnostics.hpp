/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <vector>
#include <string>
#include <memory>
#include "kio/lsp/lsp_server.hpp"
#include "kio/lsp/document_manager.hpp"

namespace kio::lsp {

enum class DiagnosticSeverity {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4
};

class DiagnosticsEngine {
public:
    DiagnosticsEngine(DocumentManager* doc_manager);
    ~DiagnosticsEngine();
    
    std::vector<Diagnostic> analyzDocument(const std::string& uri);
    
private:
    DocumentManager* doc_manager_;
    
    std::vector<Diagnostic> checkSyntaxErrors(std::shared_ptr<Document> doc);
    std::vector<Diagnostic> checkSemanticErrors(std::shared_ptr<Document> doc);
    std::vector<Diagnostic> checkStyleWarnings(std::shared_ptr<Document> doc);
    std::vector<Diagnostic> checkPerformanceHints(std::shared_ptr<Document> doc);
    
    Diagnostic createDiagnostic(const Range& range, DiagnosticSeverity severity, 
                               const std::string& message, const std::string& source = "kio");
};

} // namespace kio::lsp
