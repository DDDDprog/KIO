#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <map>

#include "axeon/lexer.hpp"
#include "axeon/parser.hpp"
#include "axeon/compiler.hpp"
#include "axeon/vm.hpp"
#include "axeon/jit_engine.hpp"

using namespace kio;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Axeon Programming Language v2.0.0" << std::endl;
        std::cout << "Usage: axeon <file.axe> [options]" << std::endl;
        std::cout << "\nOptions:" << std::endl;
        std::cout << "  --vm          Use stack-based VM (default)" << std::endl;
        std::cout << "  --interp      Use tree-walking interpreter" << std::endl;
        std::cout << "  --jit         Use JIT compilation" << std::endl;
        std::cout << "\nEnvironment:" << std::endl;
        std::cout << "  AXEON_ENGINE  Set execution engine (vm/interp/jit)" << std::endl;
        return 1;
    }

    // Get execution mode from arguments or environment
    std::string engine = "vm";  // default
    
    if (const char* env_engine = std::getenv("AXEON_ENGINE")) {
        engine = env_engine;
    }
    
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--vm") engine = "vm";
        else if (arg == "--interp") engine = "interp";
        else if (arg == "--jit") engine = "jit";
    }

    std::string filename = argv[1];
    
    // Read file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    try {
        // Lexical analysis
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.scanTokens();
        
        // Parsing
        Parser::setSourceForErrors(source, filename);
        Parser parser(tokens);
        std::vector<StmtPtr> statements = parser.parse();
        
        // Compilation
        Compiler compiler(nullptr, Compiler::FunctionType::TYPE_SCRIPT);
        ObjFunction* function = compiler.compile(statements);
        
        if (!function) {
            std::cerr << "Compilation failed" << std::endl;
            return 1;
        }
        
        // Execution
        if (engine == "vm" || engine == "default") {
            VM vm;
            InterpretResult result = vm.interpret(function);
            return (result == InterpretResult::OK) ? 0 : 1;
        } else if (engine == "jit") {
            #ifdef AXEON_JIT_ENABLED
            JITEngine jit;
            // JIT execution would go here
            VM vm;
            InterpretResult result = vm.interpret(function);
            return (result == InterpretResult::OK) ? 0 : 1;
            #else
            std::cerr << "JIT not enabled in this build" << std::endl;
            return 1;
            #endif
        } else if (engine == "interp") {
            // Interpreter execution would use AST directly
            // For now, fall back to VM
            VM vm;
            InterpretResult result = vm.interpret(function);
            return (result == InterpretResult::OK) ? 0 : 1;
        } else {
            std::cerr << "Unknown engine: " << engine << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
