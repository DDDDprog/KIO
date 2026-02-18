/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/type_system.hpp"
#include "axeon/interpreter.hpp"  // For Value definition
#include <iostream>

namespace kio {

TypeSystem::TypeSystem() {
    // Initialize built-in types
    builtin_types_[TypeKind::Void] = std::make_shared<Type>(TypeKind::Void, "void");
    builtin_types_[TypeKind::Bool] = std::make_shared<Type>(TypeKind::Bool, "bool");
    builtin_types_[TypeKind::Int] = std::make_shared<Type>(TypeKind::Int, "int");
    builtin_types_[TypeKind::Float] = std::make_shared<Type>(TypeKind::Float, "float");
    builtin_types_[TypeKind::String] = std::make_shared<Type>(TypeKind::String, "string");
    builtin_types_[TypeKind::Any] = std::make_shared<Type>(TypeKind::Any, "any");
}

std::shared_ptr<Type> TypeSystem::inferType(const std::variant<std::monostate, double, std::string> &value) {
    // Value is std::variant<std::monostate, double, std::string>
    if (std::holds_alternative<std::monostate>(value)) {
        return getVoidType();
    } else if (std::holds_alternative<double>(value)) {
        return getFloatType();
    } else if (std::holds_alternative<std::string>(value)) {
        return getStringType();
    }
    return getAnyType();
}

// inferType for ExprPtr removed - would cause forward declaration conflicts

bool TypeSystem::checkType(const std::variant<std::monostate, double, std::string> &value, const Type &expected) {
    auto inferred = inferType(value);
    return inferred->isAssignableFrom(expected);
}

void TypeSystem::registerType(const std::string &name, std::shared_ptr<Type> type) {
    registered_types_[name] = type;
}

std::shared_ptr<Type> TypeSystem::getType(const std::string &name) {
    auto it = registered_types_.find(name);
    if (it != registered_types_.end()) {
        return it->second;
    }
    
    // Check built-in types
    for (const auto& [kind, type] : builtin_types_) {
        if (type->name == name) {
            return type;
        }
    }
    
    return nullptr;
}

std::shared_ptr<Type> TypeSystem::getVoidType() {
    return builtin_types_[TypeKind::Void];
}

std::shared_ptr<Type> TypeSystem::getBoolType() {
    return builtin_types_[TypeKind::Bool];
}

std::shared_ptr<Type> TypeSystem::getIntType() {
    return builtin_types_[TypeKind::Int];
}

std::shared_ptr<Type> TypeSystem::getFloatType() {
    return builtin_types_[TypeKind::Float];
}

std::shared_ptr<Type> TypeSystem::getStringType() {
    return builtin_types_[TypeKind::String];
}

std::shared_ptr<Type> TypeSystem::getAnyType() {
    return builtin_types_[TypeKind::Any];
}

std::shared_ptr<Type> TypeSystem::getArrayType(std::shared_ptr<Type> element_type) {
    auto arrayType = std::make_shared<Type>(TypeKind::Array, "array");
    arrayType->parameters.push_back(element_type);
    return arrayType;
}

std::shared_ptr<Type> TypeSystem::getFunctionType(std::vector<std::shared_ptr<Type>> param_types, 
                                                   std::shared_ptr<Type> return_type) {
    auto funcType = std::make_shared<Type>(TypeKind::Function, "function");
    funcType->parameters = param_types;
    funcType->parameters.push_back(return_type); // Last parameter is return type
    return funcType;
}

} // namespace kio
