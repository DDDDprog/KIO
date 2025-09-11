#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include "kio/lsp/document_manager.hpp"
#include "kio/lsp/diagnostics.hpp"
#include "kio/lsp/completion.hpp"
#include "kio/lsp/hover.hpp"
#include "kio/lsp/goto_definition.hpp"
#include "kio/lsp/formatting.hpp"
#include "kio/lsp/semantic_tokens.hpp"

namespace kio::lsp {

struct Position {
    int line;
    int character;
};

struct Range {
    Position start;
    Position end;
};

struct Location {
    std::string uri;
    Range range;
};

struct TextEdit {
    Range range;
    std::string newText;
};

struct CompletionItem {
    std::string label;
    std::string detail;
    std::string documentation;
    int kind;
    std::string insertText;
};

struct Diagnostic {
    Range range;
    int severity;
    std::string message;
    std::string source;
};

struct Hover {
    std::string contents;
    Range range;
};

class LSPServer {
public:
    LSPServer();
    ~LSPServer();
    
    void start();
    void stop();
    
    // LSP Protocol Methods
    void initialize(const std::string& params);
    void initialized();
    void shutdown();
    void exit();
    
    // Document Synchronization
    void didOpen(const std::string& uri, const std::string& text);
    void didChange(const std::string& uri, const std::string& text);
    void didSave(const std::string& uri);
    void didClose(const std::string& uri);
    
    // Language Features
    std::vector<CompletionItem> completion(const std::string& uri, Position pos);
    Hover hover(const std::string& uri, Position pos);
    std::vector<Location> gotoDefinition(const std::string& uri, Position pos);
    std::vector<TextEdit> formatting(const std::string& uri);
    std::vector<int> semanticTokens(const std::string& uri);
    
private:
    std::unique_ptr<DocumentManager> document_manager_;
    std::unique_ptr<DiagnosticsEngine> diagnostics_;
    std::unique_ptr<CompletionProvider> completion_provider_;
    std::unique_ptr<HoverProvider> hover_provider_;
    std::unique_ptr<GotoDefinitionProvider> goto_provider_;
    std::unique_ptr<DocumentFormatter> formatting_provider_;
    std::unique_ptr<SemanticTokensProvider> tokens_provider_;
    
    std::thread worker_thread_;
    std::mutex request_mutex_;
    std::condition_variable request_cv_;
    std::queue<std::function<void()>> request_queue_;
    std::atomic<bool> running_{false};
    
    void processRequests();
    void sendResponse(const std::string& response);
    void sendNotification(const std::string& method, const std::string& params);
    std::string readMessage();
    void writeMessage(const std::string& message);
    
    void publishDiagnostics(const std::string& uri);
    void updateWorkspaceSymbols();
};

} // namespace kio::lsp
