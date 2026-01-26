/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include <gmp.h>
#include <iostream>
#include <string>
#include "kio/bytecode.hpp"

namespace kio {

// High-precision big int math for KIO
Value native_bigint_add(int argCount, Value* args) {
    if (argCount != 2) return Value();
    
    mpz_t a, b, res;
    mpz_init_set_str(a, std::get<std::string>(args[0].as).c_str(), 10);
    mpz_init_set_str(b, std::get<std::string>(args[1].as).c_str(), 10);
    mpz_init(res);
    
    mpz_add(res, a, b);
    
    char* s = mpz_get_str(NULL, 10, res);
    std::string result(s);
    free(s);
    
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(res);
    
    return Value(result);
}

} // namespace kio
