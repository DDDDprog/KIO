/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/lsp/lsp_server.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace kio::lsp {

LSPServer::LSPServer() 
    : document_manager_(std::make_unique<DocumentManager>())
    , diagnostics_(std::make_unique<DiagnosticsProvider>())
    , completion_provider_(std::make_unique<CompletionProvider>())
    , hover_provider_(std::make_unique<HoverProvider>())
    , goto_provider_(std::make_unique<GotoDefinitionProvider>())
    , formatting_provider_(std::make_unique<DocumentFormatter>())
    , tokens_provider_(std::make_unique<SemanticTokensProvider>())
{
}

LSPServer::~LSPServer() {
    stop();
}

void LSPServer::start() {
    running_ = true;
    worker_thread_ = std::thread(&LSPServer::processRequests, this);
    
    std::string line;
    while (running_ && std::getline(std::cin, line)) {
        if (line.find("Content-Length:") == 0) {
            size_t length = std::stoul(line.substr(15));
            std::getline(std::cin, line); // Empty line
            
            std::string content(length, '\0');
            std::cin.read(&content[0], length);
            
            // Parse and handle JSON-RPC message
            // Implementation would parse JSON and dispatch to appropriate methods
        }
    }
}

void LSPServer::stop() {
    running_ = false;
    request_cv_.notify_all();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void LSPServer::processRequests() {
    while (running_) {
        std::unique_lock<std::mutex> lock(request_mutex_);
        request_cv_.wait(lock, [this] { return !request_queue_.empty() || !running_; });
        
        while (!request_queue_.empty() && running_) {
            auto request = std::move(request_queue_.front());
            request_queue_.pop();
            lock.unlock();
            
            request();
            
            lock.lock();
        }
    }
}

std::vector<CompletionItem> LSPServer::completion(const std::string& uri, Position pos) {
    auto doc = document_manager_->get_document(uri);
    if (!doc) return {};
    return completion_provider_->get_completions(doc->content, pos);
}

Hover LSPServer::hover(const std::string& uri, Position pos) {
    auto doc = document_manager_->get_document(uri);
    if (!doc) return {"", std::nullopt};
    auto h = hover_provider_->get_hover(doc->content, pos);
    return h ? *h : Hover{"", std::nullopt};
}

void LSPServer::didOpen(const std::string& uri, const std::string& text) {
    document_manager_->open_document(uri, text);
    publishDiagnostics(uri);
}

void LSPServer::didChange(const std::string& uri, const std::string& text) {
    document_manager_->update_document(uri, text, 1);
    publishDiagnostics(uri);
}

void LSPServer::publishDiagnostics(const std::string& uri) {
    auto doc = document_manager_->get_document(uri);
    if (!doc) return;
    auto diagnostics = diagnostics_->analyze_document(doc->content);
    // Send diagnostics notification to client
}

std::vector<Location> LSPServer::gotoDefinition(const std::string& uri, Position pos) {
    auto doc = document_manager_->get_document(uri);
    if (!doc) return {};
    auto loc = goto_provider_->get_definition(doc->content, pos);
    if (loc) return {*loc};
    return {};
}

std::vector<TextEdit> LSPServer::formatting(const std::string& uri) {
    auto doc = document_manager_->get_document(uri);
    if (!doc) return {};
    FormattingOptions options; 
    return formatting_provider_->format_document(doc->content, options);
}

std::vector<uint32_t> LSPServer::semanticTokens(const std::string& uri) {
    auto doc = document_manager_->get_document(uri);
    if (!doc) return {};
    return tokens_provider_->get_semantic_tokens(doc->content);
}

} // namespace kio::lsp
