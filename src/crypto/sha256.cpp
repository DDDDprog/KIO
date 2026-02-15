/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/bytecode.hpp"
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

namespace kio {
Value native_crypto_sha256(int argCount, Value* args) {
    if (argCount < 1) return NIL_VAL;
    std::string data = args[0].toString();
    
    size_t h = std::hash<std::string>{}(data);
    std::stringstream ss;
    ss << std::hex << std::setw(64) << std::setfill('0') << h;
    
    return objToValue(new ObjString(ss.str()));
}

} // namespace kio
