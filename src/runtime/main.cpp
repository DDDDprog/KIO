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

static void run_vm(const std::string &source, VM& vm, const std::string& file_path = "") {
    if (source.empty()) return;
    try {
        Parser::setSourceForErrors(source, file_path);
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(tokens);
        auto statements = parser.parse();
        if (statements.empty()) return;

        Compiler compiler;
        ObjFunction* function = compiler.compile(statements);
        InterpretResult result = vm.interpret(function);
        if (result == InterpretResult::RUNTIME_ERROR) {
            std::cerr << "VM Runtime Error!" << std::endl;
        }
        delete function;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

static void run_interpreter(const std::string &source, Interpreter& interp, const std::string& file_path = "") {
    if (source.empty()) return;
    try {
        Parser::setSourceForErrors(source, file_path);
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
    const char *magenta = std::getenv("NO_COLOR") ? "" : "\x1b[35m";
    const char *reset = std::getenv("NO_COLOR") ? "" : "\x1b[0m";
    std::cout << cyan;
    std::cout << "    ▄▄▄      ▒██   ██▒▓█████  ▒█████   ███▄    █\n";
    std::cout << "  ▒████▄     ▒▒ █ █ ▒░▓█   ▀ ▒██▒  ██▒ ██ ▀█   █\n";
    std::cout << "  ▒██  ▀█▄   ░░  █   ░▒███   ▒██░  ██▒▓██  ▀█ ██▒\n";
    std::cout << "  ░██▄▄▄▄██   ░ █ █ ▒ ▒▓█  ▄ ▒██   ██░▓██▒  ▐▌██▒\n";
    std::cout << "   ▓█   ▓██▒ ▒██▒ ▒██▒░▒████▒░ ████▓▒░▒██░   ▓██░\n";
    std::cout << "   ▒▒   ▓▒█░ ▒▒ ░ ░▓ ░░░ ▒░ ░░ ▒░▒░▒░ ░ ▒░   ▒ ▒ \n";
    std::cout << reset;
    std::cout << "              " << magenta << "AXEON PRO" << reset << " | Version 2.1.0\n";
    std::cout << "Type :help for commands, :quit to exit\n\n";
}

static void runBenchmark() {
    std::cout << "🚀 Starting Production-Grade Benchmark (100M Iterations)..." << std::endl;
    std::string benchSource = 
        "{\n"
        "    let iterations = 100000000;\n"
        "    let sum = 0;\n"
        "    let i = 0;\n"
        "    let start = sys \"time\";\n"
        "    while (i < iterations) {\n"
        "        sum = sum + i * 2 - floor(i / 2) + (i % 3);\n"
        "        i = i + 1;\n"
        "    }\n"
        "    let endTime = sys \"time\";\n"
        "    print \"Final Sum: \" + sum;\n"
        "    print \"Total Execution Time: \" + (endTime - start) + \" ms\";\n"
        "}\n";
    VM vm;
    run_vm(benchSource, vm);
}

static void repl(EngineMode engine) {
    printLogo();
    std::string line;
    VM vm;
    Interpreter interp;
    while (true) {
        const char *yellow = std::getenv("NO_COLOR") ? "" : "\x1b[33m";
        const char *reset = std::getenv("NO_COLOR") ? "" : "\x1b[0m";
        std::cout << yellow << "axeon> " << reset;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;
        if (line[0] == ':') {
            if (line == ":quit" || line == ":q") break;
            if (line == ":clear") {
                std::cout << "\033[2J\033[1;1H";
                continue;
            }
            if (line == ":help") {
                std::cout << "\nAXEON REPL COMMANDS:\n";
                std::cout << "  :help       Show this help message\n";
                std::cout << "  :clear      Clear the terminal screen\n";
                std::cout << "  :quit, :q   Exit the REPL\n\n";
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

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--version" || arg == "-v") {
            std::cout << "Axeon 2.1.0-pro (Build 2026.02.17)\n";
            return 0;
        }
        if (arg == "--benchmark") {
            runBenchmark();
            return 0;
        }
        if (arg == "--help" || arg == "-h") {
            std::cout << "\nAXEON: THE HIGH-PERFORMANCE SCRIPTING LANGUAGE\n\n";
            std::cout << "Usage: axeon [options] [script_file]\n\n";
            std::cout << "General Options:\n";
            std::cout << "  -v, --version      Display system version and build info\n";
            std::cout << "  -h, --help         Display this detailed help message\n";
            std::cout << "  --benchmark        Run standard performance stress-test\n\n";
            std::cout << "Execution Options:\n";
            std::cout << "  --engine=vm        Run with optimized Bytecode VM (default)\n";
            std::cout << "  --engine=interp    Run with slow Tree-Walking Interpreter\n";
            std::cout << "  --no-jit           Disable JIT compilation in VM mode\n\n";
            std::cout << "Environment Variables:\n";
            std::cout << "  AXEON_ENGINE       Set to 'vm' or 'interp'\n";
            std::cout << "  NO_COLOR           Disable terminal ANSI colors\n\n";
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
        if (arg.size() > 0 && arg[0] != '-') {
            scriptPath = arg;
            break;
        }
    }

    if (!scriptPath.empty()) {
        std::string source = readFile(scriptPath);
        if (source.empty()) {
            std::cerr << "Error: Could not read file '" << scriptPath << "'" << std::endl;
            return 1;
        }
        if (engine == EngineMode::INTERPRETER) {
            Interpreter interp;
            run_interpreter(source, interp, scriptPath);
        } else {
            VM vm;
            run_vm(source, vm, scriptPath);
        }
    } else {
        repl(engine);
    }
    return 0;
}
