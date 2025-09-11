#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "kio/lexer.hpp"
#include "kio/parser.hpp"
#include "kio/interpreter.hpp"

using namespace kio;

static std::string readFile(const std::string &path) {
    std::ifstream ifs(path);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

static void run(const std::string &source) {
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(tokens);
    auto statements = parser.parse();
    Interpreter interp;
    interp.interpret(statements);
}

static void repl() {
    std::string line;
    Interpreter interp;
    while (true) {
        const char *green = std::getenv("NO_COLOR") ? "" : "\x1b[32m";
        const char *reset = std::getenv("NO_COLOR") ? "" : "\x1b[0m";
        std::cout << green << "> " << reset;
        if (!std::getline(std::cin, line)) break;
        if (!line.empty() && line[0] == ':') {
            if (line == ":quit" || line == ":q") break;
            if (line == ":help") {
                std::cout << ":help    : show this help\n:quit    : exit\n:load f  : run a file\n";
                continue;
            }
            std::cerr << "Unknown command. Try :help\n";
            continue;
        }
        try {
            Lexer lexer(line + ";");
            auto tokens = lexer.scanTokens();
            Parser parser(tokens);
            auto statements = parser.parse();
            interp.interpret(statements);
        } catch (const std::exception &e) {
            std::cerr << e.what() << "\n";
        }
    }
}

int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "--version") {
#ifdef KIO_VERSION
        std::cout << KIO_VERSION << "\n";
#else
        std::cout << "1.0.0" << "\n";
#endif
        return 0;
    }
    if (argc > 1) {
        if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            std::cout << "kio "
#ifdef KIO_VERSION
                      << KIO_VERSION
#else
                      << "1.0.0"
#endif
                      << "\nUsage: kio [file.kio] [--version] [--help]\n";
            return 0;
        }
        try {
            run(readFile(argv[1]));
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    } else {
        repl();
    }
    return 0;
}
