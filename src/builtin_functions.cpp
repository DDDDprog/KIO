#include "kio/builtin_functions.hpp"
#include <iostream>
#include <cmath>
#include <chrono>
#include <random>

namespace kio {

BuiltinFunctions::BuiltinFunctions() {
    registerBuiltins();
}

void BuiltinFunctions::registerBuiltins() {
    // I/O functions
    functions["print"] = [](const std::vector<Value>& args) -> Value {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << args[i].toString();
        }
        std::cout << std::endl;
        return Value();
    };
    
    functions["input"] = [](const std::vector<Value>& args) -> Value {
        if (!args.empty()) {
            std::cout << args[0].toString();
        }
        std::string input;
        std::getline(std::cin, input);
        return Value(input);
    };
    
    // Math functions
    functions["abs"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw std::runtime_error("abs() takes exactly 1 argument");
        return Value(std::abs(args[0].toNumber()));
    };
    
    functions["sqrt"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw std::runtime_error("sqrt() takes exactly 1 argument");
        return Value(std::sqrt(args[0].toNumber()));
    };
    
    functions["pow"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() != 2) throw std::runtime_error("pow() takes exactly 2 arguments");
        return Value(std::pow(args[0].toNumber(), args[1].toNumber()));
    };
    
    // String functions
    functions["len"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() != 1) throw std::runtime_error("len() takes exactly 1 argument");
        return Value(static_cast<double>(args[0].toString().length()));
    };
    
    functions["substr"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || args.size() > 3) {
            throw std::runtime_error("substr() takes 2 or 3 arguments");
        }
        std::string str = args[0].toString();
        size_t start = static_cast<size_t>(args[1].toNumber());
        size_t length = (args.size() == 3) ? static_cast<size_t>(args[2].toNumber()) : std::string::npos;
        return Value(str.substr(start, length));
    };
    
    // Utility functions
    functions["time"] = [](const std::vector<Value>& args) -> Value {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return Value(static_cast<double>(timestamp));
    };
    
    functions["random"] = [](const std::vector<Value>& args) -> Value {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 1.0);
        return Value(dis(gen));
    };
}

Value BuiltinFunctions::call(const std::string& name, const std::vector<Value>& args) {
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second(args);
    }
    throw std::runtime_error("Unknown builtin function: " + name);
}

bool BuiltinFunctions::exists(const std::string& name) const {
    return functions.find(name) != functions.end();
}

std::vector<std::string> BuiltinFunctions::getFunctionNames() const {
    std::vector<std::string> names;
    for (const auto& pair : functions) {
        names.push_back(pair.first);
    }
    return names;
}

} // namespace kio
