/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace kio {

class Value; // Forward declaration

using BuiltinFunction = std::function<Value(const std::vector<Value>&)>;

class BuiltinFunctions {
public:
    BuiltinFunctions();
    ~BuiltinFunctions();
    
    void registerFunction(const std::string& name, BuiltinFunction func);
    bool hasFunction(const std::string& name) const;
    Value callFunction(const std::string& name, const std::vector<Value>& args) const;
    
    std::vector<std::string> getFunctionNames() const;
    
    // Built-in function implementations
    static Value print(const std::vector<Value>& args);
    static Value println(const std::vector<Value>& args);
    static Value len(const std::vector<Value>& args);
    static Value type(const std::vector<Value>& args);
    static Value str(const std::vector<Value>& args);
    static Value int_func(const std::vector<Value>& args);
    static Value float_func(const std::vector<Value>& args);
    static Value bool_func(const std::vector<Value>& args);
    
    // Math functions
    static Value abs_func(const std::vector<Value>& args);
    static Value min_func(const std::vector<Value>& args);
    static Value max_func(const std::vector<Value>& args);
    static Value pow_func(const std::vector<Value>& args);
    static Value sqrt_func(const std::vector<Value>& args);
    
    // String functions
    static Value substr(const std::vector<Value>& args);
    static Value split(const std::vector<Value>& args);
    static Value join(const std::vector<Value>& args);
    static Value upper(const std::vector<Value>& args);
    static Value lower(const std::vector<Value>& args);

private:
    std::unordered_map<std::string, BuiltinFunction> functions_;
    
    void registerBuiltinFunctions();
};

} // namespace kio
