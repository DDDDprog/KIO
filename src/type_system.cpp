#include "kio/type_system.hpp"
#include <iostream>

namespace kio {

TypeSystem::TypeSystem() {
    registerBuiltinTypes();
}

void TypeSystem::registerBuiltinTypes() {
    // Register basic types
    registerType("void", TypeInfo{TypeKind::Void, 0, {}});
    registerType("bool", TypeInfo{TypeKind::Bool, sizeof(bool), {}});
    registerType("int", TypeInfo{TypeKind::Int, sizeof(int), {}});
    registerType("float", TypeInfo{TypeKind::Float, sizeof(double), {}});
    registerType("string", TypeInfo{TypeKind::String, sizeof(std::string), {}});
    registerType("array", TypeInfo{TypeKind::Array, sizeof(void*), {}});
    registerType("object", TypeInfo{TypeKind::Object, sizeof(void*), {}});
    registerType("function", TypeInfo{TypeKind::Function, sizeof(void*), {}});
}

void TypeSystem::registerType(const std::string& name, const TypeInfo& info) {
    types[name] = info;
}

TypeInfo* TypeSystem::getType(const std::string& name) {
    auto it = types.find(name);
    return (it != types.end()) ? &it->second : nullptr;
}

bool TypeSystem::isCompatible(const std::string& from, const std::string& to) {
    TypeInfo* fromType = getType(from);
    TypeInfo* toType = getType(to);
    
    if (!fromType || !toType) return false;
    if (from == to) return true;
    
    // Basic compatibility rules
    switch (fromType->kind) {
        case TypeKind::Int:
            return toType->kind == TypeKind::Float || toType->kind == TypeKind::String;
        case TypeKind::Float:
            return toType->kind == TypeKind::Int || toType->kind == TypeKind::String;
        case TypeKind::Bool:
            return toType->kind == TypeKind::String;
        default:
            return false;
    }
}

std::string TypeSystem::inferType(const Value& value) {
    switch (value.getType()) {
        case Value::Type::Null: return "void";
        case Value::Type::Bool: return "bool";
        case Value::Type::Number: return "float";
        case Value::Type::String: return "string";
        case Value::Type::Array: return "array";
        case Value::Type::Object: return "object";
        case Value::Type::Function: return "function";
        default: return "void";
    }
}

bool TypeSystem::checkTypes(const std::string& expected, const std::string& actual) {
    if (expected == actual) return true;
    return isCompatible(actual, expected);
}

std::vector<std::string> TypeSystem::getRegisteredTypes() const {
    std::vector<std::string> typeNames;
    for (const auto& pair : types) {
        typeNames.push_back(pair.first);
    }
    return typeNames;
}

} // namespace kio
