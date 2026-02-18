/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>

namespace kio {

enum class TypeKind {
    Void,
    Bool,
    Int,
    Float,
    String,
    Array,
    Object,
    Function,
    Class,
    Module,
    Any
};

class Type {
public:
    TypeKind kind;
    std::string name;
    std::vector<std::shared_ptr<Type>> parameters; // For generics
    
    Type(TypeKind k, const std::string &n = "") : kind(k), name(n) {}
    
    bool isAssignableFrom(const Type &other) const;
    bool isNumeric() const;
    bool isCallable() const;
    std::string toString() const;
};

class TypeSystem {
public:
    TypeSystem();
    
    // Type inference and checking
    std::shared_ptr<Type> inferType(const std::variant<std::monostate, double, std::string> &value);
    bool checkType(const std::variant<std::monostate, double, std::string> &value, const Type &expected);
    
    // Type registration
    void registerType(const std::string &name, std::shared_ptr<Type> type);
    std::shared_ptr<Type> getType(const std::string &name);
    
    // Built-in types
    std::shared_ptr<Type> getVoidType();
    std::shared_ptr<Type> getBoolType();
    std::shared_ptr<Type> getIntType();
    std::shared_ptr<Type> getFloatType();
    std::shared_ptr<Type> getStringType();
    std::shared_ptr<Type> getAnyType();
    
    // Generic types
    std::shared_ptr<Type> getArrayType(std::shared_ptr<Type> element_type);
    std::shared_ptr<Type> getFunctionType(std::vector<std::shared_ptr<Type>> param_types, 
                                         std::shared_ptr<Type> return_type);

private:
    std::unordered_map<std::string, std::shared_ptr<Type>> registered_types_;
    std::unordered_map<TypeKind, std::shared_ptr<Type>> builtin_types_;
};

} // namespace kio
