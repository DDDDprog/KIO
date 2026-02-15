/*
 * AXEON C Core ABI
 *
 * This header defines the stable C-level ABI for the AXEON VM/runtime
 * and vector engine. It is designed to be:
 *  - Pure C (C99) and embeddable from C++ (via extern "C")
 *  - Binary-stable across AXEON versions once finalized
 *  - A minimal surface that higher-level runtimes and tools can call into
 *
 * NOTE: This is a design header. The initial C implementation will
 *       be provided in the c_core/ directory and bridged from the
 *       existing C++ runtime.
 */

#ifndef AXEON_C_CORE_ABI_H
#define AXEON_C_CORE_ABI_H

#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint8_t, uint32_t */

/* Public API export/import macro. Adjust as needed for shared builds. */
#ifndef AXEON_API
#  if defined(_WIN32) && defined(AXEON_BUILD_SHARED)
#    define AXEON_API __declspec(dllexport)
#  elif defined(_WIN32)
#    define AXEON_API __declspec(dllimport)
#  else
#    define AXEON_API
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* Status / error codes                                                      */
/* ------------------------------------------------------------------------- */

typedef enum kio_status {
    AXEON_STATUS_OK = 0,
    AXEON_STATUS_COMPILE_ERROR = 1,
    AXEON_STATUS_RUNTIME_ERROR = 2,
    AXEON_STATUS_OOM = 3,
    AXEON_STATUS_INVALID_ARGUMENT = 4,
    AXEON_STATUS_INTERNAL_ERROR = 5
} kio_status;

/* Optional extended error information */
typedef struct kio_error {
    kio_status  status;     /* High-level status code             */
    uint32_t    code;       /* Domain-specific error code         */
    const char* message;    /* Optional static or heap-allocated  */
} kio_error;

/* ------------------------------------------------------------------------- */
/* Value representation                                                      */
/* ------------------------------------------------------------------------- */

typedef enum kio_value_type {
    AXEON_VAL_NUMBER = 0,
    AXEON_VAL_BOOL   = 1,
    AXEON_VAL_NIL    = 2,
    AXEON_VAL_OBJ    = 3
} kio_value_type;

/* Forward declaration for opaque heap objects managed by the runtime. */
typedef struct kio_obj kio_obj;

typedef struct kio_value {
    kio_value_type type;
    union {
        double   number;
        int32_t  boolean; /* 0 = false, non-zero = true */
        kio_obj* obj;
    } as;
} kio_value;

/* Convenient constructors (implemented in C core). */
AXEON_API kio_value kio_make_nil(void);
AXEON_API kio_value kio_make_bool(int32_t b);
AXEON_API kio_value kio_make_number(double x);
AXEON_API kio_value kio_make_obj(kio_obj* o);

/* Predicates and accessors (do not change the value). */
AXEON_API int        kio_is_number(kio_value v);
AXEON_API int        kio_is_bool  (kio_value v);
AXEON_API int        kio_is_nil   (kio_value v);
AXEON_API int        kio_is_obj   (kio_value v);
AXEON_API double     kio_as_number(kio_value v);
AXEON_API int32_t    kio_as_bool  (kio_value v);
AXEON_API kio_obj*   kio_as_obj   (kio_value v);

/* ------------------------------------------------------------------------- */
/* Bytecode / chunk representation                                          */
/* ------------------------------------------------------------------------- */

typedef enum kio_opcode {
    /* Keep numerics aligned with OpCode in include/kio/bytecode.hpp
       where possible, but treat this enum as the canonical C ABI. */
    AXEON_OP_CONSTANT = 0,
    AXEON_OP_NIL,
    AXEON_OP_TRUE,
    AXEON_OP_FALSE,
    AXEON_OP_POP,
    AXEON_OP_GET_LOCAL,
    AXEON_OP_SET_LOCAL,
    AXEON_OP_GET_GLOBAL,
    AXEON_OP_DEFINE_GLOBAL,
    AXEON_OP_SET_GLOBAL,
    AXEON_OP_ADD,
    AXEON_OP_SUBTRACT,
    AXEON_OP_MULTIPLY,
    AXEON_OP_DIVIDE,
    AXEON_OP_MODULO,
    AXEON_OP_EQUAL,
    AXEON_OP_GREATER,
    AXEON_OP_GREATER_EQUAL,
    AXEON_OP_LESS,
    AXEON_OP_LESS_EQUAL,
    AXEON_OP_NOT,
    AXEON_OP_NEGATE,
    AXEON_OP_PRINT,
    AXEON_OP_JUMP,
    AXEON_OP_JUMP_IF_FALSE,
    AXEON_OP_LOOP,
    AXEON_OP_CALL,
    AXEON_OP_ARRAY_NEW,
    AXEON_OP_ARRAY_GET,
    AXEON_OP_ARRAY_SET,
    AXEON_OP_SYS_QUERY,
    AXEON_OP_HALT
} kio_opcode;

/* Opaque handle to a chunk of bytecode owned by the C core. */
typedef struct kio_chunk kio_chunk;

/* Chunk lifecycle and emission */
AXEON_API kio_chunk* kio_chunk_new(void);
AXEON_API void       kio_chunk_free(kio_chunk* chunk);

/* Append a single byte of code. Line information is optional; pass 0 if unknown. */
AXEON_API kio_status kio_chunk_write(kio_chunk* chunk, uint8_t byte, int line);

/* Add a constant to the chunk and return its index via out_index. */
AXEON_API kio_status kio_chunk_add_constant(kio_chunk* chunk,
                                          kio_value   value,
                                          int*        out_index);

/* ------------------------------------------------------------------------- */
/* VM runtime                                                               */
/* ------------------------------------------------------------------------- */

/* Opaque VM handle. */
typedef struct kio_vm kio_vm;

/* VM creation / destruction */
AXEON_API kio_vm*    kio_vm_new(void);
AXEON_API void       kio_vm_free(kio_vm* vm);

/* Configure VM (flags, optimization levels, etc.) – extensible key/value API. */
AXEON_API kio_status kio_vm_set_option(kio_vm* vm, const char* key, const char* value);

/* Execute a chunk. Returns a status code and optional error info. */
AXEON_API kio_status kio_vm_execute(kio_vm*        vm,
                                  const kio_chunk* chunk,
                                  kio_value*      out_result,
                                  kio_error*      out_error);

/* ------------------------------------------------------------------------- */
/* Native function ABI                                                      */
/* ------------------------------------------------------------------------- */

/* Status return for native functions, allowing them to signal failures. */
typedef enum kio_native_status {
    AXEON_NATIVE_OK = 0,
    AXEON_NATIVE_ERROR = 1
} kio_native_status;

typedef struct kio_native_ctx kio_native_ctx; /* Opaque per-VM or per-call context. */

typedef kio_native_status (*kio_native_fn)(
    int           arg_count,
    const kio_value* args,
    kio_value*    out_result,
    kio_native_ctx* ctx);

/* Register a native function under a global name. */
AXEON_API kio_status kio_vm_register_native(kio_vm*        vm,
                                          const char*    name,
                                          kio_native_fn  fn,
                                          void*          user_data);

/* ------------------------------------------------------------------------- */
/* Vector engine ABI                                                        */
/* ------------------------------------------------------------------------- */

/* A lightweight, cache-friendly view over a contiguous double buffer. */
typedef struct kio_vec_view {
    double* data;
    size_t  length;
    size_t  stride; /* in elements; usually 1 */
} kio_vec_view;

/* Basic elementwise operations: out[i] = f(a[i], b[i]) */
AXEON_API kio_status kio_vec_add (const kio_vec_view* a,
                                const kio_vec_view* b,
                                kio_vec_view*       out);

AXEON_API kio_status kio_vec_sub (const kio_vec_view* a,
                                const kio_vec_view* b,
                                kio_vec_view*       out);

AXEON_API kio_status kio_vec_mul (const kio_vec_view* a,
                                const kio_vec_view* b,
                                kio_vec_view*       out);

AXEON_API kio_status kio_vec_div (const kio_vec_view* a,
                                const kio_vec_view* b,
                                kio_vec_view*       out);

/* Dot product: *out = sum_i a[i] * b[i] */
AXEON_API kio_status kio_vec_dot (const kio_vec_view* a,
                                const kio_vec_view* b,
                                double*             out);

/* Normalize: out[i] = a[i] / ||a|| */
AXEON_API kio_status kio_vec_normalize(const kio_vec_view* a,
                                     kio_vec_view*       out);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXEON_C_CORE_ABI_H */

