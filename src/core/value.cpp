/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/bytecode.hpp"
#include <iostream>

namespace kio {

Value::Value(const std::string& s) : type(ValueType::VAL_OBJ) {
    as.obj = new ObjString(s);
}

Value::Value(const char* s) : type(ValueType::VAL_OBJ) {
    as.obj = new ObjString(std::string(s));
}

bool Value::operator==(const Value& other) const {
    if (type != other.type) return false;
    switch (type) {
        case ValueType::VAL_NIL:    return true;
        case ValueType::VAL_BOOL:   return as.boolean == other.as.boolean;
        case ValueType::VAL_NUMBER: return as.number == other.as.number;
        case ValueType::VAL_NATIVE: return as.native == other.as.native;
        case ValueType::VAL_OBJ: {
            if (as.obj->type == ObjType::OBJ_STRING && other.as.obj->type == ObjType::OBJ_STRING) {
                return ((ObjString*)as.obj)->chars == ((ObjString*)other.as.obj)->chars;
            }
            return as.obj == other.as.obj;
        }
    }
    return false;
}

} // namespace kio
