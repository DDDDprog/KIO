/*
 Copyright (c) 2025 Dipanjan Dhar
 SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef KIO_ASM_HELPERS_HPP
#define KIO_ASM_HELPERS_HPP

#include <cstdint>
#include <cstddef>

namespace kio {

// Inline assembly helpers for critical arithmetic operations
// These provide direct CPU instructions for maximum performance

static inline double asm_add_double(double a, double b) {
    double result;
    __asm__ volatile (
        "movsd %1, %0\n\t"
        "addsd %2, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
    );
    return result;
}

static inline double asm_sub_double(double a, double b) {
    double result;
    __asm__ volatile (
        "movsd %1, %0\n\t"
        "subsd %2, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
    );
    return result;
}

static inline double asm_mul_double(double a, double b) {
    double result;
    __asm__ volatile (
        "movsd %1, %0\n\t"
        "mulsd %2, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
    );
    return result;
}

static inline double asm_div_double(double a, double b) {
    double result;
    __asm__ volatile (
        "movsd %1, %0\n\t"
        "divsd %2, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
    );
    return result;
}

// SIMD operations for arrays (SSE2)
static inline void asm_mul_array_double(double* dst, const double* src, size_t count) {
    size_t i = 0;
    size_t simd_width = count & ~7ULL;  // Process 8 doubles at a time
    
    for (; i < simd_width; i += 8) {
        __asm__ volatile (
            "movupd   (%1), %%xmm0\n\t"
            "movupd  16(%1), %%xmm1\n\t"
            "movupd  32(%1), %%xmm2\n\t"
            "movupd  48(%1), %%xmm3\n\t"
            "mulpd    %%xmm4, %%xmm0\n\t"
            "mulpd    %%xmm4, %%xmm1\n\t"
            "mulpd    %%xmm4, %%xmm2\n\t"
            "mulpd    %%xmm4, %%xmm3\n\t"
            "movupd   %%xmm0, (%0)\n\t"
            "movupd   %%xmm1, 16(%0)\n\t"
            "movupd   %%xmm2, 32(%0)\n\t"
            "movupd   %%xmm3, 48(%0)"
            : 
            : "r"(dst + i), "r"(src + i), "x"(2.0)
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
    }
    
    // Handle remaining elements
    for (; i < count; i++) {
        dst[i] = src[i] * 2.0;
    }
}

// Prefetch hints for better cache utilization
static inline void asm_prefetch(const void* ptr, int locality = 3) {
    __builtin_prefetch(ptr, 0, locality);
}

static inline void asm_prefetch_write(const void* ptr, int locality = 3) {
    __builtin_prefetch(ptr, 1, locality);
}

// Memory barrier for thread safety
static inline void asm_mfence() {
    __asm__ volatile ("mfence" ::: "memory");
}

static inline void asm_lfence() {
    __asm__ volatile ("lfence" ::: "memory");
}

static inline void asm_sfence() {
    __asm__ volatile ("sfence" ::: "memory");
}

// Fast branchless conditional move for doubles
static inline double asm_branchless_min(double a, double b) {
    double result;
    __asm__ (
        "minsd %1, %0"
        : "=x"(result)
        : "x"(a), "0"(b)
    );
    return result;
}

static inline double asm_branchless_max(double a, double b) {
    double result;
    __asm__ (
        "maxsd %1, %0"
        : "=x"(result)
        : "x"(a), "0"(b)
    );
    return result;
}

// Unaligned load/store for fast memory access
static inline void asm_unaligned_store(double* ptr, double value) {
    __asm__ volatile (
        "movsd %1, %0"
        : "=m"(*ptr)
        : "x"(value)
    );
}

static inline double asm_unaligned_load(const double* ptr) {
    double value;
    __asm__ volatile (
        "movsd %1, %0"
        : "=x"(value)
        : "m"(*ptr)
    );
    return value;
}

} // namespace kio

#endif // KIO_ASM_HELPERS_HPP
