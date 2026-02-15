/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: Zeo-3.0-only
*/

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include "axeon/lexer.hpp"
#include "axeon/parser.hpp"
#include "axeon/vm.hpp"
#include "axeon/compiler.hpp"
#include "axeon/interpreter.hpp"

using namespace kio;

enum class EngineMode {
    VM,
    INTERPRETER
};

static EngineMode parseEngineFromEnv() {
    const char* env = std::getenv("AXEON_ENGINE");
    if (!env) return EngineMode::VM;
    std::string v(env);
    if (v == "interp" || v == "interpreter") return EngineMode::INTERPRETER;
    return EngineMode::VM;
}

static std::string readFile(const std::string &path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) return "";
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

static void run_vm(const std::string &source, VM& vm) {
    if (source.empty()) return;
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(tokens);
        auto statements = parser.parse();
        if (statements.empty()) return;

        Compiler compiler;
        Chunk* chunk = compiler.compile(statements);
        InterpretResult result = vm.interpret(chunk);
        if (result == InterpretResult::RUNTIME_ERROR) {
            std::cerr << "VM Runtime Error!" << std::endl;
        }
        delete chunk;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

static void run_interpreter(const std::string &source, Interpreter& interp) {
    if (source.empty()) return;
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(tokens);
        auto statements = parser.parse();
        if (statements.empty()) return;

        interp.interpret(statements);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

static void printLogo() {
    const char *cyan = std::getenv("NO_COLOR") ? "" : "\x1b[36m";
    const char *reset = std::getenv("NO_COLOR") ? "" : "\x1b[0m";
    std::cout << cyan;
    std::cout << "    ▄▄▄      ▒██   ██▒▓█████  ▒█████   ███▄    █\n";
    std::cout << "  ▒████▄     ▒▒ █ █ ▒░▓█   ▀ ▒██▒  ██▒ ██ ▀█   █\n";
    std::cout << "  ▒██  ▀█▄   ░░  █   ░▒███   ▒██░  ██▒▓██  ▀█ ██▒\n";
    std::cout << "  ░██▄▄▄▄██   ░ █ █ ▒ ▒▓█  ▄ ▒██   ██░▓██▒  ▐▌██▒\n";
    std::cout << "   ▓█   ▓██▒ ▒██▒ ▒██▒░▒████▒░ ████▓▒░▒██░   ▓██░\n";
    std::cout << "   ▒▒   ▓▒█░ ▒▒ ░ ░▓ ░░░ ▒░ ░░ ▒░▒░▒░ ░ ▒░   ▒ ▒ \n";
    std::cout << reset;
    std::cout << "              AXEON LANG\n";
    std::cout << "Type :help for commands, :quit to exit\n\n";
}

static void repl(EngineMode engine) {
    printLogo();
    std::string line;
    VM vm;
    Interpreter interp;
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
        if (engine == EngineMode::INTERPRETER) {
            run_interpreter(line + ";", interp);
        } else {
            run_vm(line + ";", vm);
        }
    }
}

int main(int argc, char **argv) {
    EngineMode engine = parseEngineFromEnv();
    std::string scriptPath;

    // Parse CLI flags (may override engine from env).
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--version") {
            std::cout << "2.0.0\n";
            return 0;
        }
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: kio [options] [file.kio]\n";
            std::cout << "Options:\n";
            std::cout << "  --version          Show version\n";
            std::cout << "  --help, -h         Show this help\n";
            std::cout << "  --engine=vm        Use bytecode VM (default)\n";
            std::cout << "  --engine=interp    Use tree-walking interpreter\n";
            return 0;
        }
        if (arg.rfind("--engine=", 0) == 0) {
            std::string mode = arg.substr(std::string("--engine=").size());
            if (mode == "interp" || mode == "interpreter") {
                engine = EngineMode::INTERPRETER;
            } else {
                engine = EngineMode::VM;
            }
            continue;
        }
        // First non-flag argument is treated as script path.
        if (arg.size() > 0 && arg[0] != '-') {
            scriptPath = arg;
            break;
        }
    }

    if (!scriptPath.empty()) {
        std::string source = readFile(scriptPath);
        if (engine == EngineMode::INTERPRETER) {
            Interpreter interp;
            run_interpreter(source, interp);
        } else {
            VM vm;
            run_vm(source, vm);
        }
    } else {
        repl(engine);
    }
    return 0;
}
