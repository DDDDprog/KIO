#pragma once

#include <vector>
#include <string>
#include <memory>
#include "kio/lsp/lsp_server.hpp"
#include "kio/lsp/document_manager.hpp"

namespace kio::lsp {

enum class CompletionItemKind {
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18
};

class CompletionProvider {
public:
    CompletionProvider(DocumentManager* doc_manager);
    ~CompletionProvider();
    
    std::vector<CompletionItem> provideCompletion(const std::string& uri, Position pos);
    
private:
    DocumentManager* doc_manager_;
    
    std::vector<CompletionItem> getKeywordCompletions();
    std::vector<CompletionItem> getBuiltinCompletions();
    std::vector<CompletionItem> getVariableCompletions(std::shared_ptr<Document> doc, Position pos);
    std::vector<CompletionItem> getFunctionCompletions(std::shared_ptr<Document> doc, Position pos);
    std::vector<CompletionItem> getModuleCompletions(std::shared_ptr<Document> doc, Position pos);
    std::vector<CompletionItem> getSnippetCompletions();
    
    CompletionItem createCompletion(const std::string& label, CompletionItemKind kind,
                                   const std::string& detail = "", const std::string& docs = "",
                                   const std::string& insertText = "");
};

} // namespace kio::lsp
