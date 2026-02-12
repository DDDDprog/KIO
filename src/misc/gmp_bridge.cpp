/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include <gmp.h>
#include <iostream>
#include <string>
#include "kio/bytecode.hpp"

namespace kio {

static std::string getString(Value v) {
    if (isObj(v)) {
        Obj* o = valueToObj(v);
        if (o->type == ObjType::OBJ_STRING) {
            return ((ObjString*)o)->chars;
        }
    }
    return "";
}

Value native_bigint_add(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    
    mpz_t a, b, res;
    mpz_init_set_str(a, getString(args[0]).c_str(), 10);
    mpz_init_set_str(b, getString(args[1]).c_str(), 10);
    mpz_init(res);
    
    mpz_add(res, a, b);
    
    char* s = mpz_get_str(NULL, 10, res);
    std::string result(s);
    free(s);
    
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(res);
    
    return objToValue(new ObjString(result));
}

Value native_bigint_sub(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    
    mpz_t a, b, res;
    mpz_init_set_str(a, getString(args[0]).c_str(), 10);
    mpz_init_set_str(b, getString(args[1]).c_str(), 10);
    mpz_init(res);
    
    mpz_sub(res, a, b);
    
    char* s = mpz_get_str(NULL, 10, res);
    std::string result(s);
    free(s);
    
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(res);
    
    return objToValue(new ObjString(result));
}

Value native_bigint_mul(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    
    mpz_t a, b, res;
    mpz_init_set_str(a, getString(args[0]).c_str(), 10);
    mpz_init_set_str(b, getString(args[1]).c_str(), 10);
    mpz_init(res);
    
    mpz_mul(res, a, b);
    
    char* s = mpz_get_str(NULL, 10, res);
    std::string result(s);
    free(s);
    
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(res);
    
    return objToValue(new ObjString(result));
}

Value native_bigint_div(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    
    mpz_t a, b, res;
    mpz_init_set_str(a, getString(args[0]).c_str(), 10);
    mpz_init_set_str(b, getString(args[1]).c_str(), 10);
    
    if (mpz_sgn(b) == 0) {
        mpz_clear(a);
        mpz_clear(b);
        return NIL_VAL; 
    }
    
    mpz_init(res);
    mpz_tdiv_q(res, a, b);
    
    char* s = mpz_get_str(NULL, 10, res);
    std::string result(s);
    free(s);
    
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(res);
    
    return objToValue(new ObjString(result));
}

Value native_bigint_mod(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    
    mpz_t a, b, res;
    mpz_init_set_str(a, getString(args[0]).c_str(), 10);
    mpz_init_set_str(b, getString(args[1]).c_str(), 10);
    
    if (mpz_sgn(b) == 0) {
        mpz_clear(a);
        mpz_clear(b);
        return NIL_VAL;
    }
    
    mpz_init(res);
    mpz_tdiv_r(res, a, b);
    
    char* s = mpz_get_str(NULL, 10, res);
    std::string result(s);
    free(s);
    
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(res);
    
    return objToValue(new ObjString(result));
}

Value native_bigint_pow(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    
    mpz_t a, res;
    mpz_init_set_str(a, getString(args[0]).c_str(), 10);
    unsigned long exp = std::stoul(getString(args[1]));
    mpz_init(res);
    
    mpz_pow_ui(res, a, exp);
    
    char* s = mpz_get_str(NULL, 10, res);
    std::string result(s);
    free(s);
    
    mpz_clear(a);
    mpz_clear(res);
    
    return objToValue(new ObjString(result));
}

Value native_bigint_cmp(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    
    mpz_t a, b;
    mpz_init_set_str(a, getString(args[0]).c_str(), 10);
    mpz_init_set_str(b, getString(args[1]).c_str(), 10);
    
    int cmp = mpz_cmp(a, b);
    
    mpz_clear(a);
    mpz_clear(b);
    
    return doubleToValue((double)cmp);
}

} // namespace kio
