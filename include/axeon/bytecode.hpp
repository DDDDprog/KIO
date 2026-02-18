/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <map>
#include <memory>

namespace kio {

enum class OpCode : uint8_t {
    CONSTANT = 0, NIL, TRUE, FALSE, POP,
    GET_LOCAL, SET_LOCAL, GET_GLOBAL, DEFINE_GLOBAL, SET_GLOBAL,
    ADD, SUBTRACT, MULTIPLY, DIVIDE, MODULO,
    EQUAL, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
    NOT, NEGATE, PRINT, JUMP, JUMP_IF_FALSE, LOOP,
    CALL, INVOKE, RETURN, // Logic
    CLASS, METHOD, GET_PROPERTY, SET_PROPERTY, INHERIT, // OOP
    ARRAY_NEW, ARRAY_GET, ARRAY_SET, SYS_QUERY,
    FLOOR, SQRT,
    // Fast native loop for benchmarks
    FAST_LOOP,
    HALT
};

enum class ValueType { VAL_NUMBER, VAL_BOOL, VAL_NIL, VAL_OBJ };

struct Obj;

#define QNAN     ((uint64_t)0x7ff8000000000000)
#define SIGN_BIT ((uint64_t)0x8000000000000000)

struct Value {
    uint64_t v;

    Value() : v(((uint64_t)(0x7ff8000000000000) | 1)) {} // NIL
    Value(double n);
    Value(bool b) : v(((uint64_t)(0x7ff8000000000000) | (b ? 3 : 2))) {} // TRUE/FALSE
    Value(Obj* o) : v((uint64_t)(0x8000000000000000 | 0x7ff8000000000000 | (uint64_t)(uintptr_t)o)) {}
    Value(const std::string& s);
    Value(const char* s);
    
    // Explicit internal constructor
    explicit Value(uint64_t raw) : v(raw) {}

    double toNumber() const;
    std::string toString() const;

    bool operator==(const Value& other) const;
};

static inline bool isNumber(Value v) { return (v.v & 0x7ff8000000000000) != 0x7ff8000000000000; }
static inline bool isNil(Value v)    { return v.v == ((uint64_t)(0x7ff8000000000000) | 1); }
static inline bool isBool(Value v)   { return (v.v & ~1) == ((uint64_t)(0x7ff8000000000000) | 2); }
static inline bool isObj(Value v)    { return (v.v & (0x8000000000000000 | 0x7ff8000000000000)) == (0x8000000000000000 | 0x7ff8000000000000); }

static inline double valueToDouble(Value v) {
    union { uint64_t u; double d; } cast;
    cast.u = v.v;
    return cast.d;
}
static inline Value doubleToValue(double d) { return Value(d); }
static inline Obj* valueToObj(Value v) { return (Obj*)(uintptr_t)(v.v & ~(0x8000000000000000 | 0x7ff8000000000000)); }
static inline Value objToValue(Obj* o) { return Value(o); }

#define NIL_VAL   Value()
#define FALSE_VAL Value(false)
#define TRUE_VAL  Value(true)
#define BOOL_VAL(b) Value(b)

enum class ObjType { OBJ_STRING, OBJ_ARRAY, OBJ_FUNCTION, OBJ_CLASS, OBJ_INSTANCE };
struct Obj {
    ObjType type;
    virtual ~Obj() = default;
protected: Obj(ObjType t) : type(t) {}
};

struct ObjString : public Obj {
    std::string chars;
    ObjString(const std::string& s) : Obj(ObjType::OBJ_STRING), chars(s) {}
};

struct ObjArray : public Obj {
    std::vector<Value> elements;
    ObjArray() : Obj(ObjType::OBJ_ARRAY) {}
};

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    int addConstant(Value v) { constants.push_back(v); return constants.size()-1; }
    void write(uint8_t b, int l) { code.push_back(b); }
};

struct ObjFunction : public Obj {
    int arity;
    Chunk chunk;
    std::string name;
    ObjFunction() : Obj(ObjType::OBJ_FUNCTION), arity(0) {}
};

struct ObjClass : public Obj {
    std::string name;
    std::map<std::string, Value> methods;
    ObjClass(const std::string& n) : Obj(ObjType::OBJ_CLASS), name(n) {}
};

struct ObjInstance : public Obj {
    ObjClass* klass;
    std::map<std::string, Value> fields;
    ObjInstance(ObjClass* k) : Obj(ObjType::OBJ_INSTANCE), klass(k) {}
};

} // namespace kio
