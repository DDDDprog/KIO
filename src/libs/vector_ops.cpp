/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include <vector>
#include <immintrin.h>
#include <cmath>
#include <algorithm>
#include "axeon/bytecode.hpp"

namespace kio {

void vectorized_add(ObjArray* a, ObjArray* b, ObjArray* result) {
    size_t size = std::min({a->elements.size(), b->elements.size(), result->elements.size()});
    size_t i = 0;
#ifdef __AVX2__
    for (; i + 4 <= size; i += 4) {
        __m256d va = _mm256_set_pd(valueToDouble(a->elements[i+3]), valueToDouble(a->elements[i+2]), valueToDouble(a->elements[i+1]), valueToDouble(a->elements[i]));
        __m256d vb = _mm256_set_pd(valueToDouble(b->elements[i+3]), valueToDouble(b->elements[i+2]), valueToDouble(b->elements[i+1]), valueToDouble(b->elements[i]));
        double res[4]; _mm256_storeu_pd(res, _mm256_add_pd(va, vb));
        for(int j=0; j<4; ++j) result->elements[i+j] = doubleToValue(res[j]);
    }
#endif
    for (; i < size; ++i) {
        if (isNumber(a->elements[i]) && isNumber(b->elements[i])) {
            result->elements[i] = doubleToValue(valueToDouble(a->elements[i]) + valueToDouble(b->elements[i]));
        }
    }
}

void vectorized_sub(ObjArray* a, ObjArray* b, ObjArray* result) {
    size_t size = std::min({a->elements.size(), b->elements.size(), result->elements.size()});
    size_t i = 0;
#ifdef __AVX2__
    for (; i + 4 <= size; i += 4) {
        __m256d va = _mm256_set_pd(valueToDouble(a->elements[i+3]), valueToDouble(a->elements[i+2]), valueToDouble(a->elements[i+1]), valueToDouble(a->elements[i]));
        __m256d vb = _mm256_set_pd(valueToDouble(b->elements[i+3]), valueToDouble(b->elements[i+2]), valueToDouble(b->elements[i+1]), valueToDouble(b->elements[i]));
        double res[4]; _mm256_storeu_pd(res, _mm256_sub_pd(va, vb));
        for(int j=0; j<4; ++j) result->elements[i+j] = doubleToValue(res[j]);
    }
#endif
    for (; i < size; ++i) {
        if (isNumber(a->elements[i]) && isNumber(b->elements[i])) {
            result->elements[i] = doubleToValue(valueToDouble(a->elements[i]) - valueToDouble(b->elements[i]));
        }
    }
}

void vectorized_mul(ObjArray* a, ObjArray* b, ObjArray* result) {
    size_t size = std::min({a->elements.size(), b->elements.size(), result->elements.size()});
    size_t i = 0;
#ifdef __AVX2__
    for (; i + 4 <= size; i += 4) {
        __m256d va = _mm256_set_pd(valueToDouble(a->elements[i+3]), valueToDouble(a->elements[i+2]), valueToDouble(a->elements[i+1]), valueToDouble(a->elements[i]));
        __m256d vb = _mm256_set_pd(valueToDouble(b->elements[i+3]), valueToDouble(b->elements[i+2]), valueToDouble(b->elements[i+1]), valueToDouble(b->elements[i]));
        double res[4]; _mm256_storeu_pd(res, _mm256_mul_pd(va, vb));
        for(int j=0; j<4; ++j) result->elements[i+j] = doubleToValue(res[j]);
    }
#endif
    for (; i < size; ++i) {
        if (isNumber(a->elements[i]) && isNumber(b->elements[i])) {
            result->elements[i] = doubleToValue(valueToDouble(a->elements[i]) * valueToDouble(b->elements[i]));
        }
    }
}

void vectorized_div(ObjArray* a, ObjArray* b, ObjArray* result) {
    size_t size = std::min({a->elements.size(), b->elements.size(), result->elements.size()});
    size_t i = 0;
#ifdef __AVX2__
    for (; i + 4 <= size; i += 4) {
        __m256d va = _mm256_set_pd(valueToDouble(a->elements[i+3]), valueToDouble(a->elements[i+2]), valueToDouble(a->elements[i+1]), valueToDouble(a->elements[i]));
        __m256d vb = _mm256_set_pd(valueToDouble(b->elements[i+3]), valueToDouble(b->elements[i+2]), valueToDouble(b->elements[i+1]), valueToDouble(b->elements[i]));
        double res[4]; _mm256_storeu_pd(res, _mm256_div_pd(va, vb));
        for(int j=0; j<4; ++j) result->elements[i+j] = doubleToValue(res[j]);
    }
#endif
    for (; i < size; ++i) {
        if (isNumber(a->elements[i]) && isNumber(b->elements[i]) && valueToDouble(b->elements[i]) != 0) {
            result->elements[i] = doubleToValue(valueToDouble(a->elements[i]) / valueToDouble(b->elements[i]));
        }
    }
}

double vectorized_dot(ObjArray* a, ObjArray* b) {
    size_t size = std::min(a->elements.size(), b->elements.size());
    double total = 0.0;
    size_t i = 0;
#ifdef __AVX2__
    __m256d sum = _mm256_setzero_pd();
    for (; i + 4 <= size; i += 4) {
        __m256d va = _mm256_set_pd(valueToDouble(a->elements[i+3]), valueToDouble(a->elements[i+2]), valueToDouble(a->elements[i+1]), valueToDouble(a->elements[i]));
        __m256d vb = _mm256_set_pd(valueToDouble(b->elements[i+3]), valueToDouble(b->elements[i+2]), valueToDouble(b->elements[i+1]), valueToDouble(b->elements[i]));
        sum = _mm256_add_pd(sum, _mm256_mul_pd(va, vb));
    }
    double res[4];
    _mm256_storeu_pd(res, sum);
    total = res[0] + res[1] + res[2] + res[3];
#endif
    for (; i < size; ++i) {
        if (isNumber(a->elements[i]) && isNumber(b->elements[i])) {
            total += valueToDouble(a->elements[i]) * valueToDouble(b->elements[i]);
        }
    }
    return total;
}

void vectorized_cross(ObjArray* a, ObjArray* b, ObjArray* result) {
    if (a->elements.size() < 3 || b->elements.size() < 3 || result->elements.size() < 3) return;
    double a1 = valueToDouble(a->elements[0]);
    double a2 = valueToDouble(a->elements[1]);
    double a3 = valueToDouble(a->elements[2]);
    double b1 = valueToDouble(b->elements[0]);
    double b2 = valueToDouble(b->elements[1]);
    double b3 = valueToDouble(b->elements[2]);

    result->elements[0] = doubleToValue(a2 * b3 - a3 * b2);
    result->elements[1] = doubleToValue(a3 * b1 - a1 * b3);
    result->elements[2] = doubleToValue(a1 * b2 - a2 * b1);
}

void vectorized_normalize(ObjArray* a, ObjArray* result) {
    size_t size = std::min(a->elements.size(), result->elements.size());
    double mag_sq = 0;
    for (size_t i = 0; i < size; ++i) {
        double val = valueToDouble(a->elements[i]);
        mag_sq += val * val;
    }
    double mag = std::sqrt(mag_sq);
    if (mag == 0) return;
    for (size_t i = 0; i < size; ++i) {
        result->elements[i] = doubleToValue(valueToDouble(a->elements[i]) / mag);
    }
}

} // namespace kio
