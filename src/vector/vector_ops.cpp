/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include <vector>
#include <immintrin.h>
#include <cmath>
#include <algorithm>
#include "kio/bytecode.hpp"

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

} // namespace kio
