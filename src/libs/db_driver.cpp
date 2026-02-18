/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include <unordered_map>
#include <fstream>
#include <string>
#include "axeon/bytecode.hpp"

namespace kio {

class KioDB {
    std::unordered_map<std::string, std::string> store;
public:
    void set(const std::string& key, const std::string& value) { store[key] = value; }
    std::string get(const std::string& key) { return store.count(key) ? store[key] : ""; }
};

static KioDB g_db;

Value native_db_set(int argCount, Value* args) {
    if (argCount == 2 && isObj(args[0]) && isObj(args[1])) {
        g_db.set(((ObjString*)valueToObj(args[0]))->chars, ((ObjString*)valueToObj(args[1]))->chars);
    }
    return NIL_VAL;
}

Value native_db_get(int argCount, Value* args) {
    if (argCount == 1 && isObj(args[0])) {
        return objToValue(new ObjString(g_db.get(((ObjString*)valueToObj(args[0]))->chars)));
    }
    return objToValue(new ObjString(""));
}

} // namespace kio
