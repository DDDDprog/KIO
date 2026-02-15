/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/type_system.hpp"

namespace kio {

bool Type::isAssignableFrom(const Type &other) const {
    if (kind == other.kind) return true;
    if (kind == TypeKind::Any) return true;
    if (kind == TypeKind::Float && other.kind == TypeKind::Int) return true;
    return false;
}

bool Type::isNumeric() const {
    return kind == TypeKind::Int || kind == TypeKind::Float;
}

bool Type::isCallable() const {
    return kind == TypeKind::Function;
}

std::string Type::toString() const {
    if (!name.empty()) return name;
    
    switch (kind) {
        case TypeKind::Void: return "void";
        case TypeKind::Bool: return "bool";
        case TypeKind::Int: return "int";
        case TypeKind::Float: return "float";
        case TypeKind::String: return "string";
        case TypeKind::Array: return "array";
        case TypeKind::Object: return "object";
        case TypeKind::Function: return "function";
        case TypeKind::Class: return "class";
        case TypeKind::Module: return "module";
        case TypeKind::Any: return "any";
    }
    return "unknown";
}

} // namespace kio
