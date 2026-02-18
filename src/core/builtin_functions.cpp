/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/builtin_functions.hpp"
#include "axeon/bytecode.hpp"
#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <algorithm>
#include <thread>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include "axeon/network/http_server.hpp"

namespace kio {

BuiltinFunctions::BuiltinFunctions() {
    registerBuiltinFunctions();
}

BuiltinFunctions::~BuiltinFunctions() {}

void BuiltinFunctions::registerFunction(const std::string& name, BuiltinFunction func) {
    functions_[name] = func;
}

bool BuiltinFunctions::hasFunction(const std::string& name) const {
    return functions_.find(name) != functions_.end();
}

Value BuiltinFunctions::callFunction(const std::string& name, const std::vector<Value>& args) const {
    auto it = functions_.find(name);
    if (it != functions_.end()) {
        return it->second(args);
    }
    return NIL_VAL;
}

std::vector<std::string> BuiltinFunctions::getFunctionNames() const {
    std::vector<std::string> names;
    for (const auto& pair : functions_) {
        names.push_back(pair.first);
    }
    return names;
}

// Native bridge externs
extern Value native_bigint_add(int argCount, Value* args);
extern Value native_bigint_sub(int argCount, Value* args);
extern Value native_bigint_mul(int argCount, Value* args);
extern Value native_bigint_div(int argCount, Value* args);
extern Value native_bigint_mod(int argCount, Value* args);
extern Value native_bigint_pow(int argCount, Value* args);
extern Value native_bigint_cmp(int argCount, Value* args);

extern Value native_crypto_aes_encrypt(int argCount, Value* args);
extern Value native_crypto_aes_decrypt(int argCount, Value* args);
extern Value native_crypto_sha256(int argCount, Value* args);

extern Value native_gpu_init(int argCount, Value* args);
extern Value native_gpu_malloc(int argCount, Value* args);
extern Value native_gpu_sync(int argCount, Value* args);

extern Value native_gui_window(int argCount, Value* args);
extern Value native_gui_button(int argCount, Value* args);

// Vectorized ops
extern void vectorized_add(ObjArray* a, ObjArray* b, ObjArray* result);
extern void vectorized_sub(ObjArray* a, ObjArray* b, ObjArray* result);
extern void vectorized_mul(ObjArray* a, ObjArray* b, ObjArray* result);
extern void vectorized_div(ObjArray* a, ObjArray* b, ObjArray* result);
extern double vectorized_dot(ObjArray* a, ObjArray* b);
extern void vectorized_cross(ObjArray* a, ObjArray* b, ObjArray* result);
extern void vectorized_normalize(ObjArray* a, ObjArray* result);

void BuiltinFunctions::registerBuiltinFunctions() {
    // Register implementations
    functions_["print"] = print;
    functions_["println"] = println;
    functions_["floor"] = floor_func;
    functions_["len"] = len;
    functions_["type"] = type;
    functions_["str"] = str;
    functions_["abs"] = abs_func;
    functions_["min"] = min_func;
    functions_["max"] = max_func;
    functions_["pow"] = pow_func;
    functions_["sqrt"] = sqrt_func;
    functions_["substr"] = substr;

    // Helper to register stubs for un-implemented functions
    auto registerStub = [&](const std::string& name) {
        if (functions_.find(name) == functions_.end()) {
            functions_[name] = [name](const std::vector<Value>&) {
                std::cerr << "Built-in function '" << name << "' is not yet implemented." << std::endl;
                return NIL_VAL;
            };
        }
    };

    const std::vector<std::string> stubs = {
        "input", "format", "to_string", "to_int", "to_float", "to_bool", "to_array",
        "range", "map", "filter", "reduce", "zip", "enumerate", "join", "split", "trim",
        "upper", "lower", "replace", "contains", "startswith", "endswith", "index_of",
        "char_at", "substring", "to_upper", "to_lower", "reverse", "sort", "unique",
        "flatten", "flat_map", "for_each", "any", "all", "none", "sum", "avg", "product",
        "count", "first", "last", "take", "drop", "skip", "take_while", "drop_while",
        "partition", "group_by", "into_iter", "iter", "iter_mut", "collect",
        "ceil", "round", "sin", "cos", "tan", "asin", "acos", "atan", "atan2",
        "log", "log10", "exp", "sign", "rand", "rand_int", "rand_float", "rand_bool", "seed",
        "time", "sleep", "timestamp", "datetime", "format_time", "exit", "panic", "assert",
        "debug", "trace", "env", "args", "open", "read", "read_line", "write", "write_line",
        "close", "exists", "mkdir", "remove", "rename", "copy", "http_get", "http_post",
        "http_put", "http_delete", "http_request", "json_parse", "json_stringify",
        "json_to_map", "json_to_array", "sha256", "sha512", "md5", "aes_encrypt", "aes_decrypt",
        "base64_encode", "base64_decode", "hash", "db_connect", "db_query", "db_execute",
        "db_close", "tcp_connect", "tcp_listen", "udp_send", "udp_receive", "clone", "ref",
        "move", "drop", "size_of", "align_of", "some", "none", "is_some", "is_none",
        "unwrap", "unwrap_or", "map_or", "ok", "err", "is_ok", "is_err", "vec", "vector",
        "with_capacity", "push", "pop", "insert", "remove_at"
    };

    for (const auto& name : stubs) registerStub(name);

    // Override some stubs with actual implementations/wrappers if they exist
    functions_["sha256"] = [](const std::vector<Value>& args) { return native_crypto_sha256(args.size(), const_cast<Value*>(args.data())); };
    functions_["aes_encrypt"] = [](const std::vector<Value>& args) { return native_crypto_aes_encrypt(args.size(), const_cast<Value*>(args.data())); };
    functions_["aes_decrypt"] = [](const std::vector<Value>& args) { return native_crypto_aes_decrypt(args.size(), const_cast<Value*>(args.data())); };
    
    // Math module functions
    functions_["sin"] = sin_func;
    functions_["cos"] = cos_func;
    functions_["tan"] = tan_func;
    functions_["asin"] = asin_func;
    functions_["acos"] = acos_func;
    functions_["atan"] = atan_func;
    functions_["atan2"] = atan2_func;
    functions_["ceil"] = ceil_func;
    functions_["round"] = round_func;
    functions_["log"] = log_func;
    functions_["log10"] = log10_func;
    functions_["exp"] = exp_func;
    functions_["sign"] = sign_func;
    
    // Time module functions
    functions_["time"] = time_func;
    functions_["sleep"] = sleep_func;
    functions_["timestamp"] = timestamp_func;
    
    // Random module functions
    functions_["rand"] = rand_func;
    functions_["rand_int"] = rand_int_func;
    functions_["rand_float"] = rand_float_func;
    
    // String module functions
    functions_["upper"] = upper_func;
    functions_["lower"] = lower_func;
    functions_["trim"] = trim_func;
    functions_["replace"] = replace_func;
    functions_["contains"] = contains_func;
    functions_["startswith"] = startswith_func;
    functions_["endswith"] = endswith_func;
    functions_["split"] = split_func;
    functions_["join"] = join_func;
    
    // Array module functions
    functions_["range"] = range_func;
    functions_["map"] = map_func;
    functions_["filter"] = filter_func;
    functions_["reduce"] = reduce_func;
    functions_["sum"] = sum_func;
    functions_["avg"] = avg_func;
    functions_["sort"] = sort_func;
    functions_["reverse"] = reverse_func;
    
    // File module functions
    functions_["read_file"] = read_file_func;
    functions_["write_file"] = write_file_func;
    functions_["exists"] = exists_func;
    functions_["list_dir"] = list_dir_func;
}

Value BuiltinFunctions::print(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        std::cout << args[i].toString() << (i == args.size() - 1 ? "" : " ");
    }
    return NIL_VAL;
}

Value BuiltinFunctions::println(const std::vector<Value>& args) {
    print(args);
    std::cout << std::endl;
    return NIL_VAL;
}

Value BuiltinFunctions::len(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    if (isObj(args[0])) {
        Obj* o = valueToObj(args[0]);
        if (o->type == ObjType::OBJ_STRING) return doubleToValue(((ObjString*)o)->chars.length());
        if (o->type == ObjType::OBJ_ARRAY) return doubleToValue(((ObjArray*)o)->elements.size());
    }
    return doubleToValue(0);
}

Value BuiltinFunctions::type(const std::vector<Value>& args) {
    if (args.empty()) return objToValue(new ObjString("nil"));
    if (isNil(args[0])) return objToValue(new ObjString("nil"));
    if (isBool(args[0])) return objToValue(new ObjString("bool"));
    if (isNumber(args[0])) return objToValue(new ObjString("number"));
    if (isObj(args[0])) {
        Obj* o = valueToObj(args[0]);
        if (o->type == ObjType::OBJ_STRING) return objToValue(new ObjString("string"));
        if (o->type == ObjType::OBJ_ARRAY) return objToValue(new ObjString("array"));
        return objToValue(new ObjString("object"));
    }
    return objToValue(new ObjString("unknown"));
}

Value BuiltinFunctions::str(const std::vector<Value>& args) {
    if (args.empty()) return objToValue(new ObjString(""));
    return objToValue(new ObjString(args[0].toString()));
}

Value BuiltinFunctions::abs_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::abs(args[0].toNumber()));
}

Value BuiltinFunctions::min_func(const std::vector<Value>& args) {
    if (args.empty()) return NIL_VAL;
    double m = args[0].toNumber();
    for (size_t i = 1; i < args.size(); ++i) {
        m = std::min(m, args[i].toNumber());
    }
    return doubleToValue(m);
}

Value BuiltinFunctions::max_func(const std::vector<Value>& args) {
    if (args.empty()) return NIL_VAL;
    double m = args[0].toNumber();
    for (size_t i = 1; i < args.size(); ++i) {
        m = std::max(m, args[i].toNumber());
    }
    return doubleToValue(m);
}

Value BuiltinFunctions::pow_func(const std::vector<Value>& args) {
    if (args.size() < 2) return doubleToValue(0);
    return doubleToValue(std::pow(args[0].toNumber(), args[1].toNumber()));
}

Value BuiltinFunctions::sqrt_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::sqrt(args[0].toNumber()));
}

// Math functions implementation
Value BuiltinFunctions::sin_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::sin(args[0].toNumber()));
}

Value BuiltinFunctions::cos_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::cos(args[0].toNumber()));
}

Value BuiltinFunctions::tan_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::tan(args[0].toNumber()));
}

Value BuiltinFunctions::asin_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::asin(args[0].toNumber()));
}

Value BuiltinFunctions::acos_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::acos(args[0].toNumber()));
}

Value BuiltinFunctions::atan_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::atan(args[0].toNumber()));
}

Value BuiltinFunctions::atan2_func(const std::vector<Value>& args) {
    if (args.size() < 2) return doubleToValue(0);
    return doubleToValue(std::atan2(args[0].toNumber(), args[1].toNumber()));
}

Value BuiltinFunctions::ceil_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::ceil(args[0].toNumber()));
}

Value BuiltinFunctions::round_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::round(args[0].toNumber()));
}

Value BuiltinFunctions::log_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::log(args[0].toNumber()));
}

Value BuiltinFunctions::log10_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::log10(args[0].toNumber()));
}

Value BuiltinFunctions::exp_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::exp(args[0].toNumber()));
}

Value BuiltinFunctions::sign_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    double val = args[0].toNumber();
    if (val > 0) return doubleToValue(1);
    if (val < 0) return doubleToValue(-1);
    return doubleToValue(0);
}

// Time functions implementation
Value BuiltinFunctions::time_func(const std::vector<Value>& args) {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return doubleToValue(millis / 1000.0);
}

Value BuiltinFunctions::sleep_func(const std::vector<Value>& args) {
    if (args.empty()) return NIL_VAL;
    double seconds = args[0].toNumber();
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
    return NIL_VAL;
}

Value BuiltinFunctions::timestamp_func(const std::vector<Value>& args) {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return doubleToValue(millis);
}

// Random functions implementation
static std::random_device rd;
static std::mt19937 gen(rd());

Value BuiltinFunctions::rand_func(const std::vector<Value>& args) {
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return doubleToValue(dis(gen));
}

Value BuiltinFunctions::rand_int_func(const std::vector<Value>& args) {
    if (args.empty()) {
        std::uniform_int_distribution<> dis(0, RAND_MAX);
        return doubleToValue(dis(gen));
    }
    int max_val = static_cast<int>(args[0].toNumber());
    std::uniform_int_distribution<> dis(0, max_val);
    return doubleToValue(dis(gen));
}

Value BuiltinFunctions::rand_float_func(const std::vector<Value>& args) {
    if (args.empty()) {
        std::uniform_real_distribution<> dis(0.0, 1.0);
        return doubleToValue(dis(gen));
    }
    double max_val = args[0].toNumber();
    std::uniform_real_distribution<> dis(0.0, max_val);
    return doubleToValue(dis(gen));
}

// String functions implementation
Value BuiltinFunctions::upper_func(const std::vector<Value>& args) {
    if (args.empty()) return objToValue(new ObjString(""));
    std::string s = args[0].toString();
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return objToValue(new ObjString(s));
}

Value BuiltinFunctions::lower_func(const std::vector<Value>& args) {
    if (args.empty()) return objToValue(new ObjString(""));
    std::string s = args[0].toString();
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return objToValue(new ObjString(s));
}

Value BuiltinFunctions::trim_func(const std::vector<Value>& args) {
    if (args.empty()) return objToValue(new ObjString(""));
    std::string s = args[0].toString();
    // Simple trim - remove leading/trailing whitespace
    auto start = s.find_first_not_of(" \\t\\n\\r");
    auto end = s.find_last_not_of(" \\t\\n\\r");
    if (start == std::string::npos) return objToValue(new ObjString(""));
    return objToValue(new ObjString(s.substr(start, end - start + 1)));
}

Value BuiltinFunctions::replace_func(const std::vector<Value>& args) {
    if (args.size() < 3) return args.empty() ? objToValue(new ObjString("")) : args[0];
    std::string s = args[0].toString();
    std::string from = args[1].toString();
    std::string to = args[2].toString();
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.length(), to);
        pos += to.length();
    }
    return objToValue(new ObjString(s));
}

Value BuiltinFunctions::contains_func(const std::vector<Value>& args) {
    if (args.size() < 2) return BOOL_VAL(false);
    std::string s = args[0].toString();
    std::string sub = args[1].toString();
    return BOOL_VAL(s.find(sub) != std::string::npos);
}

Value BuiltinFunctions::startswith_func(const std::vector<Value>& args) {
    if (args.size() < 2) return BOOL_VAL(false);
    std::string s = args[0].toString();
    std::string prefix = args[1].toString();
    return BOOL_VAL(s.rfind(prefix, 0) == 0);
}

Value BuiltinFunctions::endswith_func(const std::vector<Value>& args) {
    if (args.size() < 2) return BOOL_VAL(false);
    std::string s = args[0].toString();
    std::string suffix = args[1].toString();
    if (s.length() >= suffix.length()) {
        return BOOL_VAL(s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0);
    }
    return BOOL_VAL(false);
}

Value BuiltinFunctions::split_func(const std::vector<Value>& args) {
    if (args.empty()) return objToValue(new ObjArray());
    std::string s = args[0].toString();
    std::string delim = args.size() > 1 ? args[1].toString() : " ";
    
    auto* arr = new ObjArray();
    size_t start = 0;
    size_t end = s.find(delim);
    while (end != std::string::npos) {
        arr->elements.push_back(objToValue(new ObjString(s.substr(start, end - start))));
        start = end + delim.length();
        end = s.find(delim, start);
    }
    arr->elements.push_back(objToValue(new ObjString(s.substr(start))));
    return objToValue(arr);
}

Value BuiltinFunctions::join_func(const std::vector<Value>& args) {
    if (args.empty()) return objToValue(new ObjString(""));
    std::string delim = args.size() > 1 ? args[1].toString() : " ";
    
    std::ostringstream result;
    bool first = true;
    
    if (isObj(args[0])) {
        Obj* o = valueToObj(args[0]);
        if (o->type == ObjType::OBJ_ARRAY) {
            ObjArray* arr = (ObjArray*)o;
            for (const auto& elem : arr->elements) {
                if (!first) result << delim;
                result << elem.toString();
                first = false;
            }
        }
    }
    return objToValue(new ObjString(result.str()));
}

// Array functions implementation
Value BuiltinFunctions::range_func(const std::vector<Value>& args) {
    int start = 0, end = 0, step = 1;
    
    if (args.empty()) return objToValue(new ObjArray());
    if (args.size() == 1) {
        end = static_cast<int>(args[0].toNumber());
    } else if (args.size() >= 2) {
        start = static_cast<int>(args[0].toNumber());
        end = static_cast<int>(args[1].toNumber());
        if (args.size() >= 3) step = static_cast<int>(args[2].toNumber());
    }
    
    auto* arr = new ObjArray();
    for (int i = start; i < end; i += step) {
        arr->elements.push_back(doubleToValue(i));
    }
    return objToValue(arr);
}

Value BuiltinFunctions::sum_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    double sum = 0;
    if (isObj(args[0])) {
        Obj* o = valueToObj(args[0]);
        if (o->type == ObjType::OBJ_ARRAY) {
            ObjArray* arr = (ObjArray*)o;
            for (const auto& elem : arr->elements) {
                sum += elem.toNumber();
            }
        }
    }
    return doubleToValue(sum);
}

Value BuiltinFunctions::avg_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    double sum = 0;
    int count = 0;
    if (isObj(args[0])) {
        Obj* o = valueToObj(args[0]);
        if (o->type == ObjType::OBJ_ARRAY) {
            ObjArray* arr = (ObjArray*)o;
            for (const auto& elem : arr->elements) {
                sum += elem.toNumber();
                count++;
            }
        }
    }
    return count > 0 ? doubleToValue(sum / count) : doubleToValue(0);
}

Value BuiltinFunctions::sort_func(const std::vector<Value>& args) {
    if (args.empty() || !isObj(args[0])) return args.empty() ? objToValue(new ObjArray()) : args[0];
    Obj* o = valueToObj(args[0]);
    if (o->type == ObjType::OBJ_ARRAY) {
        ObjArray* arr = (ObjArray*)o;
        std::vector<double> nums;
        for (const auto& elem : arr->elements) {
            nums.push_back(elem.toNumber());
        }
        std::sort(nums.begin(), nums.end());
        arr->elements.clear();
        for (double n : nums) {
            arr->elements.push_back(doubleToValue(n));
        }
    }
    return args[0];
}

Value BuiltinFunctions::reverse_func(const std::vector<Value>& args) {
    if (args.empty() || !isObj(args[0])) return args.empty() ? objToValue(new ObjArray()) : args[0];
    Obj* o = valueToObj(args[0]);
    if (o->type == ObjType::OBJ_ARRAY) {
        ObjArray* arr = (ObjArray*)o;
        std::reverse(arr->elements.begin(), arr->elements.end());
    }
    return args[0];
}

// Stub implementations for map, filter, reduce
Value BuiltinFunctions::map_func(const std::vector<Value>& args) {
    if (args.empty() || !isObj(args[0])) return objToValue(new ObjArray());
    // Return the array as-is for now (higher-order functions require lambda support)
    return args[0];
}

Value BuiltinFunctions::filter_func(const std::vector<Value>& args) {
    if (args.empty() || !isObj(args[0])) return objToValue(new ObjArray());
    return args[0];
}

Value BuiltinFunctions::reduce_func(const std::vector<Value>& args) {
    if (args.empty()) return NIL_VAL;
    return args[0];
}

// File functions implementation
Value BuiltinFunctions::read_file_func(const std::vector<Value>& args) {
    if (args.empty()) return objToValue(new ObjString(""));
    std::string filename = args[0].toString();
    std::ifstream file(filename);
    if (!file.is_open()) return objToValue(new ObjString(""));
    std::stringstream buffer;
    buffer << file.rdbuf();
    return objToValue(new ObjString(buffer.str()));
}

Value BuiltinFunctions::write_file_func(const std::vector<Value>& args) {
    if (args.size() < 2) return BOOL_VAL(false);
    std::string filename = args[0].toString();
    std::string content = args[1].toString();
    std::ofstream file(filename);
    if (!file.is_open()) return BOOL_VAL(false);
    file << content;
    file.close();
    return BOOL_VAL(true);
}

Value BuiltinFunctions::exists_func(const std::vector<Value>& args) {
    if (args.empty()) return BOOL_VAL(false);
    std::string filename = args[0].toString();
    return BOOL_VAL(std::filesystem::exists(filename));
}

Value BuiltinFunctions::list_dir_func(const std::vector<Value>& args) {
    if (args.empty()) return objToValue(new ObjArray());
    std::string path = args[0].toString();
    auto* arr = new ObjArray();
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            arr->elements.push_back(objToValue(new ObjString(entry.path().filename().string())));
        }
    } catch (...) {
        // Return empty array on error
    }
    return objToValue(arr);
}

Value BuiltinFunctions::substr(const std::vector<Value>& args) {
    if (args.size() < 2) return NIL_VAL;
    std::string s = args[0].toString();
    int start = (int)args[1].toNumber();
    int len = (args.size() > 2) ? (int)args[2].toNumber() : (int)s.length() - start;
    if (start < 0) start = 0;
    if (len < 0) len = 0;
    if (start >= (int)s.length()) return objToValue(new ObjString(""));
    return objToValue(new ObjString(s.substr(start, len)));
}

Value BuiltinFunctions::floor_func(const std::vector<Value>& args) {
    if (args.empty()) return doubleToValue(0);
    return doubleToValue(std::floor(args[0].toNumber()));
}

} // namespace kio
