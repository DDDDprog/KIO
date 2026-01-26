/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <vector>
#include <string>
#include <memory>
#include "kio/lsp/types.hpp"
#include "kio/lsp/document_manager.hpp"

namespace kio::lsp {

class CompletionProvider {
public:
    CompletionProvider();
    ~CompletionProvider();
    
    std::vector<CompletionItem> get_completions(const std::string& content, const Position& position);
    
private:
    std::vector<CompletionItem> builtin_completions_;
    
    void initialize_builtin_completions();
    CompletionContext analyze_context(const std::string& content, const Position& position);
    
    void add_keyword_completions(const CompletionContext& context, std::vector<CompletionItem>& completions);
    void add_builtin_completions(const CompletionContext& context, std::vector<CompletionItem>& completions);
    void add_variable_completions(const CompletionContext& context, std::vector<CompletionItem>& completions);
    void add_module_completions(const CompletionContext& context, std::vector<CompletionItem>& completions);
};

} // namespace kio::lsp
