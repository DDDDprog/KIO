/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include "kio/lexer.hpp"
#include "kio/parser.hpp"
#include "kio/vm.hpp"
#include "kio/compiler.hpp"

using namespace kio;

static std::string readFile(const std::string &path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) return "";
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

static void run(const std::string &source, VM& vm) {
    if (source.empty()) return;
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(tokens);
        auto statements = parser.parse();
        if (statements.empty()) return;

        Compiler compiler;
        Chunk* chunk = compiler.compile(statements);
        vm.interpret(chunk);
        delete chunk;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

static void printLogo() {
    const char *cyan = std::getenv("NO_COLOR") ? "" : "\x1b[36m";
    const char *reset = std::getenv("NO_COLOR") ? "" : "\x1b[0m";
    std::cout << cyan;
    std::cout << "╦╔═╦╔═╗\n";
    std::cout << "╠╩╗║║ ║\n";
    std::cout << "╩ ╩╩╚═╝\n";
    std::cout << reset;
    std::cout << "KIO 2.0.0\n";
    std::cout << "Type :help for commands, :quit to exit\n\n";
}

static void repl() {
    printLogo();
    std::string line;
    VM vm;
    while (true) {
        const char *green = std::getenv("NO_COLOR") ? "" : "\x1b[32m";
        const char *reset = std::getenv("NO_COLOR") ? "" : "\x1b[0m";
        std::cout << green << "> " << reset;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;
        if (line[0] == ':') {
            if (line == ":quit" || line == ":q") break;
            if (line == ":help") {
                std::cout << ":help    : show this help\n:quit    : exit\n";
                continue;
            }
            std::cerr << "Unknown command. Try :help\n";
            continue;
        }
        run(line + ";", vm);
    }
}

int main(int argc, char **argv) {
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--version") {
            std::cout << "2.0.0\n";
            return 0;
        }
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: kio [file.kio] [--version] [--help]\n";
            return 0;
        }
        VM vm;
        run(readFile(arg), vm);
    } else {
        repl();
    }
    return 0;
}
