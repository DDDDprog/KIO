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

namespace kio {

enum class OpCode : uint8_t {
    CONSTANT = 0, NIL, TRUE, FALSE, POP,
    GET_LOCAL, SET_LOCAL, GET_GLOBAL, DEFINE_GLOBAL, SET_GLOBAL,
    ADD, SUBTRACT, MULTIPLY, DIVIDE, MODULO,
    EQUAL, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
    NOT, NEGATE, PRINT, JUMP, JUMP_IF_FALSE, LOOP,
    CALL, ARRAY_NEW, ARRAY_GET, ARRAY_SET, SYS_QUERY,
    HALT
};

enum class ValueType { VAL_NUMBER, VAL_BOOL, VAL_NIL, VAL_OBJ };

struct Obj;

struct Value {
    ValueType type;
    union {
        double number;
        bool boolean;
        Obj* obj;
    } as;

    Value();
    Value(double n);
    Value(bool b);
    Value(Obj* o);
    Value(const std::string& s);
    Value(const char* s);

    double toNumber() const;
    std::string toString() const;

    bool operator==(const Value& other) const;
};

static inline bool isNumber(Value v) { return v.type == ValueType::VAL_NUMBER; }
static inline bool isNil(Value v)    { return v.type == ValueType::VAL_NIL; }
static inline bool isBool(Value v)   { return v.type == ValueType::VAL_BOOL; }
static inline bool isObj(Value v)    { return v.type == ValueType::VAL_OBJ; }

static inline double valueToDouble(Value v) { return v.as.number; }
static inline Value doubleToValue(double d) { return Value(d); }
static inline Obj* valueToObj(Value v) { return v.as.obj; }
static inline Value objToValue(Obj* o) { return Value(o); }

#define NIL_VAL   Value()
#define FALSE_VAL Value(false)
#define TRUE_VAL  Value(true)
#define BOOL_VAL(b) Value(b)

enum class ObjType { OBJ_STRING, OBJ_ARRAY };
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

} // namespace kio
