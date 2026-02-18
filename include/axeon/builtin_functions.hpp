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

    static Value floor_func(const std::vector<Value>& args);
    
    // Math functions
    static Value sin_func(const std::vector<Value>& args);
    static Value cos_func(const std::vector<Value>& args);
    static Value tan_func(const std::vector<Value>& args);
    static Value asin_func(const std::vector<Value>& args);
    static Value acos_func(const std::vector<Value>& args);
    static Value atan_func(const std::vector<Value>& args);
    static Value atan2_func(const std::vector<Value>& args);
    static Value ceil_func(const std::vector<Value>& args);
    static Value round_func(const std::vector<Value>& args);
    static Value log_func(const std::vector<Value>& args);
    static Value log10_func(const std::vector<Value>& args);
    static Value exp_func(const std::vector<Value>& args);
    static Value sign_func(const std::vector<Value>& args);
    
    // Time functions
    static Value time_func(const std::vector<Value>& args);
    static Value sleep_func(const std::vector<Value>& args);
    static Value timestamp_func(const std::vector<Value>& args);
    
    // Random functions
    static Value rand_func(const std::vector<Value>& args);
    static Value rand_int_func(const std::vector<Value>& args);
    static Value rand_float_func(const std::vector<Value>& args);
    
    // String functions
    static Value trim_func(const std::vector<Value>& args);
    static Value replace_func(const std::vector<Value>& args);
    static Value contains_func(const std::vector<Value>& args);
    static Value startswith_func(const std::vector<Value>& args);
    static Value endswith_func(const std::vector<Value>& args);
    static Value split_func(const std::vector<Value>& args);
    static Value join_func(const std::vector<Value>& args);
    
    // Array functions
    static Value range_func(const std::vector<Value>& args);
    static Value map_func(const std::vector<Value>& args);
    static Value filter_func(const std::vector<Value>& args);
    static Value reduce_func(const std::vector<Value>& args);
    static Value sum_func(const std::vector<Value>& args);
    static Value avg_func(const std::vector<Value>& args);
    static Value sort_func(const std::vector<Value>& args);
    static Value reverse_func(const std::vector<Value>& args);
    
    // File functions
    static Value read_file_func(const std::vector<Value>& args);
    static Value write_file_func(const std::vector<Value>& args);
    static Value exists_func(const std::vector<Value>& args);
    static Value list_dir_func(const std::vector<Value>& args);
    
private:
    std::unordered_map<std::string, BuiltinFunction> functions_;
    
    void registerBuiltinFunctions();
};

} // namespace kio
