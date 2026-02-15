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
    functions_["print"] = print;
    functions_["println"] = println;
    functions_["len"] = len;
    functions_["type"] = type;
    functions_["str"] = str;
    functions_["abs"] = abs_func;
    functions_["min"] = min_func;
    functions_["max"] = max_func;
    functions_["pow"] = pow_func;
    functions_["sqrt"] = sqrt_func;
    functions_["substr"] = substr;

    // GMP BigInt
    functions_["bigint_add"] = [](const std::vector<Value>& args) { return native_bigint_add(args.size(), const_cast<Value*>(args.data())); };
    functions_["bigint_sub"] = [](const std::vector<Value>& args) { return native_bigint_sub(args.size(), const_cast<Value*>(args.data())); };
    functions_["bigint_mul"] = [](const std::vector<Value>& args) { return native_bigint_mul(args.size(), const_cast<Value*>(args.data())); };
    functions_["bigint_div"] = [](const std::vector<Value>& args) { return native_bigint_div(args.size(), const_cast<Value*>(args.data())); };
    functions_["bigint_mod"] = [](const std::vector<Value>& args) { return native_bigint_mod(args.size(), const_cast<Value*>(args.data())); };
    functions_["bigint_pow"] = [](const std::vector<Value>& args) { return native_bigint_pow(args.size(), const_cast<Value*>(args.data())); };
    functions_["bigint_cmp"] = [](const std::vector<Value>& args) { return native_bigint_cmp(args.size(), const_cast<Value*>(args.data())); };
    
    // Crypto
    functions_["aes_encrypt"] = [](const std::vector<Value>& args) { return native_crypto_aes_encrypt(args.size(), const_cast<Value*>(args.data())); };
    functions_["aes_decrypt"] = [](const std::vector<Value>& args) { return native_crypto_aes_decrypt(args.size(), const_cast<Value*>(args.data())); };
    functions_["sha256"] = [](const std::vector<Value>& args) { return native_crypto_sha256(args.size(), const_cast<Value*>(args.data())); };

    // GPU
    functions_["gpu_init"] = [](const std::vector<Value>& args) { return native_gpu_init(args.size(), const_cast<Value*>(args.data())); };
    functions_["gpu_malloc"] = [](const std::vector<Value>& args) { return native_gpu_malloc(args.size(), const_cast<Value*>(args.data())); };
    functions_["gpu_sync"] = [](const std::vector<Value>& args) { return native_gpu_sync(args.size(), const_cast<Value*>(args.data())); };

    // GUI
    functions_["gui_window"] = [](const std::vector<Value>& args) { return native_gui_window(args.size(), const_cast<Value*>(args.data())); };
    functions_["gui_button"] = [](const std::vector<Value>& args) { return native_gui_button(args.size(), const_cast<Value*>(args.data())); };

    // Vectorized Ops
    functions_["vec_add"] = [](const std::vector<Value>& args) {
        if (args.size() < 3) return NIL_VAL;
        vectorized_add((ObjArray*)valueToObj(args[0]), (ObjArray*)valueToObj(args[1]), (ObjArray*)valueToObj(args[2]));
        return args[2];
    };
    functions_["vec_dot"] = [](const std::vector<Value>& args) {
        if (args.size() < 2) return doubleToValue(0);
        return doubleToValue(vectorized_dot((ObjArray*)valueToObj(args[0]), (ObjArray*)valueToObj(args[1])));
    };
    functions_["vec_cross"] = [](const std::vector<Value>& args) {
        if (args.size() < 3) return NIL_VAL;
        vectorized_cross((ObjArray*)valueToObj(args[0]), (ObjArray*)valueToObj(args[1]), (ObjArray*)valueToObj(args[2]));
        return args[2];
    };

    // Network (Http Server)
    functions_["server_init"] = [](const std::vector<Value>& args) { return native_server_init(args.size(), const_cast<Value*>(args.data())); };
    functions_["server_accept"] = [](const std::vector<Value>& args) { return native_server_accept(args.size(), const_cast<Value*>(args.data())); };
    functions_["server_respond"] = [](const std::vector<Value>& args) { return native_server_respond(args.size(), const_cast<Value*>(args.data())); };
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
    switch (args[0].type) {
        case ValueType::VAL_NIL:    return objToValue(new ObjString("nil"));
        case ValueType::VAL_BOOL:   return objToValue(new ObjString("bool"));
        case ValueType::VAL_NUMBER: return objToValue(new ObjString("number"));
        case ValueType::VAL_OBJ: {
            Obj* o = valueToObj(args[0]);
            if (o->type == ObjType::OBJ_STRING) return objToValue(new ObjString("string"));
            if (o->type == ObjType::OBJ_ARRAY) return objToValue(new ObjString("array"));
            return objToValue(new ObjString("object"));
        }
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

} // namespace kio
