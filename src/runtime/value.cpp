/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/bytecode.hpp"
#include <iostream>

namespace kio {

Value::Value(double n) {
    union { uint64_t u; double d; } cast;
    cast.d = n;
    v = cast.u;
}

Value::Value(const std::string& s) {
    Obj* o = new ObjString(s);
    v = (uint64_t)(0x8000000000000000 | 0x7ff8000000000000 | (uintptr_t)o);
}

Value::Value(const char* s) {
    Obj* o = new ObjString(std::string(s));
    v = (uint64_t)(0x8000000000000000 | 0x7ff8000000000000 | (uintptr_t)o);
}

double Value::toNumber() const {
    if (isNumber(*this)) return valueToDouble(*this);
    if (isBool(*this)) return (v == (0x7ff8000000000000 | 3)) ? 1.0 : 0.0;
    return 0.0;
}

std::string Value::toString() const {
    if (isNumber(*this)) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.15g", valueToDouble(*this));
        return std::string(buf);
    }
    if (isNil(*this)) return "nil";
    if (isBool(*this)) return (v == (0x7ff8000000000000 | 3)) ? "true" : "false";
    if (isObj(*this)) {
        Obj* o = valueToObj(*this);
        if (!o) return "nil";
        if (o->type == ObjType::OBJ_STRING) return ((ObjString*)o)->chars;
        if (o->type == ObjType::OBJ_ARRAY) return "[Array]";
        if (o->type == ObjType::OBJ_FUNCTION) return "<fn " + ((ObjFunction*)o)->name + ">";
        if (o->type == ObjType::OBJ_CLASS) return "<class " + ((ObjClass*)o)->name + ">";
        if (o->type == ObjType::OBJ_INSTANCE) return "<instance of " + ((ObjInstance*)o)->klass->name + ">";
        return "[Object]";
    }
    return "unknown";
}

bool Value::operator==(const Value& other) const {
    if (v == other.v) return true;
    if (isObj(*this) && isObj(other)) {
        Obj* o1 = valueToObj(*this);
        Obj* o2 = valueToObj(other);
        if (!o1 || !o2) return o1 == o2;
        if (o1->type == ObjType::OBJ_STRING && o2->type == ObjType::OBJ_STRING) {
            return ((ObjString*)o1)->chars == ((ObjString*)o2)->chars;
        }
    }
    return false;
}

} // namespace kio
