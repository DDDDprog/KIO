/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/bytecode.hpp"
#include <iostream>

namespace kio {

Value::Value() : type(ValueType::VAL_NIL) {
    as.obj = nullptr;
}

Value::Value(double n) : type(ValueType::VAL_NUMBER) {
    as.number = n;
}

Value::Value(bool b) : type(ValueType::VAL_BOOL) {
    as.boolean = b;
}

Value::Value(Obj* o) : type(ValueType::VAL_OBJ) {
    as.obj = o;
}

Value::Value(const std::string& s) : type(ValueType::VAL_OBJ) {
    as.obj = new ObjString(s);
}

Value::Value(const char* s) : type(ValueType::VAL_OBJ) {
    as.obj = new ObjString(std::string(s));
}

double Value::toNumber() const {
    if (type == ValueType::VAL_NUMBER) return as.number;
    if (type == ValueType::VAL_BOOL) return as.boolean ? 1.0 : 0.0;
    return 0.0;
}

std::string Value::toString() const {
    switch (type) {
        case ValueType::VAL_NIL:    return "nil";
        case ValueType::VAL_BOOL:   return as.boolean ? "true" : "false";
        case ValueType::VAL_NUMBER: {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.15g", as.number);
            return std::string(buf);
        }
        case ValueType::VAL_OBJ: {
            if (!as.obj) return "nil";
            if (as.obj->type == ObjType::OBJ_STRING) return ((ObjString*)as.obj)->chars;
            if (as.obj->type == ObjType::OBJ_ARRAY) return "[Array]";
            return "[Object]";
        }
    }
    return "";
}

bool Value::operator==(const Value& other) const {
    if (type != other.type) return false;
    switch (type) {
        case ValueType::VAL_NIL:    return true;
        case ValueType::VAL_BOOL:   return as.boolean == other.as.boolean;
        case ValueType::VAL_NUMBER: return as.number == other.as.number;
        case ValueType::VAL_OBJ: {
            if (as.obj == other.as.obj) return true;
            if (!as.obj || !other.as.obj) return false;
            if (as.obj->type == ObjType::OBJ_STRING && other.as.obj->type == ObjType::OBJ_STRING) {
                return ((ObjString*)as.obj)->chars == ((ObjString*)other.as.obj)->chars;
            }
            return false;
        }
    }
    return false;
}

} // namespace kio
