/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include <iostream>
#include <string>
#include <memory>
#include "kio/lsp/lsp_server.hpp"

int main(int argc, char* argv[]) {
    try {
        auto server = std::make_unique<kio::lsp::LSPServer>();
        
        // Handle command line arguments
        bool stdio_mode = true;
        int port = 0;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--stdio") {
                stdio_mode = true;
            } else if (arg == "--port" && i + 1 < argc) {
                port = std::stoi(argv[++i]);
                stdio_mode = false;
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "KIO Language Server Protocol\n";
                std::cout << "Usage: kio-lsp [options]\n";
                std::cout << "Options:\n";
                std::cout << "  --stdio     Use stdio for communication (default)\n";
                std::cout << "  --port N    Use TCP port N for communication\n";
                std::cout << "  --help      Show this help message\n";
                return 0;
            }
        }
        
        std::cerr << "KIO LSP Server starting..." << std::endl;
        
        if (stdio_mode) {
            std::cerr << "Using stdio communication" << std::endl;
        } else {
            std::cerr << "Using TCP port " << port << std::endl;
        }
        
        server->start();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
