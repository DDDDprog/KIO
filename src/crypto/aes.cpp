/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/bytecode.hpp"
#include <vector>
#include <string>
#include <cstring>

namespace kio {

// Basic AES-like block encryption placeholder
// In a real implementation, this would use OpenSSL or a dedicated AES library.
Value native_crypto_aes_encrypt(int argCount, Value* args) {
    if (argCount < 2) return NIL_VAL;
    std::string data = args[0].toString();
    std::string key = args[1].toString();
    
    // Safety: avoid modulo-by-zero and degenerate keys.
    if (key.empty()) {
        // With an empty key, return the input unchanged rather than
        // invoking undefined behavior.
        return objToValue(new ObjString(data));
    }
    
    std::string result = data;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] ^= key[i % key.length()];
    }
    
    return objToValue(new ObjString(result));
}

Value native_crypto_aes_decrypt(int argCount, Value* args) {
    // For XOR it's the same
    return native_crypto_aes_encrypt(argCount, args);
}

} // namespace kio
