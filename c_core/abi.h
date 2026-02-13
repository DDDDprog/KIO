/*
 * KIO C Core ABI
 *
 * This header defines the stable C-level ABI for the KIO VM/runtime
 * and vector engine. It is designed to be:
 *  - Pure C (C99) and embeddable from C++ (via extern "C")
 *  - Binary-stable across KIO versions once finalized
 *  - A minimal surface that higher-level runtimes and tools can call into
 *
 * NOTE: This is a design header. The initial C implementation will
 *       be provided in the c_core/ directory and bridged from the
 *       existing C++ runtime.
 */

#ifndef KIO_C_CORE_ABI_H
#define KIO_C_CORE_ABI_H

#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint8_t, uint32_t */

/* Public API export/import macro. Adjust as needed for shared builds. */
#ifndef KIO_API
#  if defined(_WIN32) && defined(KIO_BUILD_SHARED)
#    define KIO_API __declspec(dllexport)
#  elif defined(_WIN32)
#    define KIO_API __declspec(dllimport)
#  else
#    define KIO_API
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* Status / error codes                                                      */
/* ------------------------------------------------------------------------- */

typedef enum kio_status {
    KIO_STATUS_OK = 0,
    KIO_STATUS_COMPILE_ERROR = 1,
    KIO_STATUS_RUNTIME_ERROR = 2,
    KIO_STATUS_OOM = 3,
    KIO_STATUS_INVALID_ARGUMENT = 4,
    KIO_STATUS_INTERNAL_ERROR = 5
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
    KIO_VAL_NUMBER = 0,
    KIO_VAL_BOOL   = 1,
    KIO_VAL_NIL    = 2,
    KIO_VAL_OBJ    = 3
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
KIO_API kio_value kio_make_nil(void);
KIO_API kio_value kio_make_bool(int32_t b);
KIO_API kio_value kio_make_number(double x);
KIO_API kio_value kio_make_obj(kio_obj* o);

/* Predicates and accessors (do not change the value). */
KIO_API int        kio_is_number(kio_value v);
KIO_API int        kio_is_bool  (kio_value v);
KIO_API int        kio_is_nil   (kio_value v);
KIO_API int        kio_is_obj   (kio_value v);
KIO_API double     kio_as_number(kio_value v);
KIO_API int32_t    kio_as_bool  (kio_value v);
KIO_API kio_obj*   kio_as_obj   (kio_value v);

/* ------------------------------------------------------------------------- */
/* Bytecode / chunk representation                                          */
/* ------------------------------------------------------------------------- */

typedef enum kio_opcode {
    /* Keep numerics aligned with OpCode in include/kio/bytecode.hpp
       where possible, but treat this enum as the canonical C ABI. */
    KIO_OP_CONSTANT = 0,
    KIO_OP_NIL,
    KIO_OP_TRUE,
    KIO_OP_FALSE,
    KIO_OP_POP,
    KIO_OP_GET_LOCAL,
    KIO_OP_SET_LOCAL,
    KIO_OP_GET_GLOBAL,
    KIO_OP_DEFINE_GLOBAL,
    KIO_OP_SET_GLOBAL,
    KIO_OP_ADD,
    KIO_OP_SUBTRACT,
    KIO_OP_MULTIPLY,
    KIO_OP_DIVIDE,
    KIO_OP_MODULO,
    KIO_OP_EQUAL,
    KIO_OP_GREATER,
    KIO_OP_GREATER_EQUAL,
    KIO_OP_LESS,
    KIO_OP_LESS_EQUAL,
    KIO_OP_NOT,
    KIO_OP_NEGATE,
    KIO_OP_PRINT,
    KIO_OP_JUMP,
    KIO_OP_JUMP_IF_FALSE,
    KIO_OP_LOOP,
    KIO_OP_CALL,
    KIO_OP_ARRAY_NEW,
    KIO_OP_ARRAY_GET,
    KIO_OP_ARRAY_SET,
    KIO_OP_SYS_QUERY,
    KIO_OP_HALT
} kio_opcode;

/* Opaque handle to a chunk of bytecode owned by the C core. */
typedef struct kio_chunk kio_chunk;

/* Chunk lifecycle and emission */
KIO_API kio_chunk* kio_chunk_new(void);
KIO_API void       kio_chunk_free(kio_chunk* chunk);

/* Append a single byte of code. Line information is optional; pass 0 if unknown. */
KIO_API kio_status kio_chunk_write(kio_chunk* chunk, uint8_t byte, int line);

/* Add a constant to the chunk and return its index via out_index. */
KIO_API kio_status kio_chunk_add_constant(kio_chunk* chunk,
                                          kio_value   value,
                                          int*        out_index);

/* ------------------------------------------------------------------------- */
/* VM runtime                                                               */
/* ------------------------------------------------------------------------- */

/* Opaque VM handle. */
typedef struct kio_vm kio_vm;

/* VM creation / destruction */
KIO_API kio_vm*    kio_vm_new(void);
KIO_API void       kio_vm_free(kio_vm* vm);

/* Configure VM (flags, optimization levels, etc.) – extensible key/value API. */
KIO_API kio_status kio_vm_set_option(kio_vm* vm, const char* key, const char* value);

/* Execute a chunk. Returns a status code and optional error info. */
KIO_API kio_status kio_vm_execute(kio_vm*        vm,
                                  const kio_chunk* chunk,
                                  kio_value*      out_result,
                                  kio_error*      out_error);

/* ------------------------------------------------------------------------- */
/* Native function ABI                                                      */
/* ------------------------------------------------------------------------- */

/* Status return for native functions, allowing them to signal failures. */
typedef enum kio_native_status {
    KIO_NATIVE_OK = 0,
    KIO_NATIVE_ERROR = 1
} kio_native_status;

typedef struct kio_native_ctx kio_native_ctx; /* Opaque per-VM or per-call context. */

typedef kio_native_status (*kio_native_fn)(
    int           arg_count,
    const kio_value* args,
    kio_value*    out_result,
    kio_native_ctx* ctx);

/* Register a native function under a global name. */
KIO_API kio_status kio_vm_register_native(kio_vm*        vm,
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
KIO_API kio_status kio_vec_add (const kio_vec_view* a,
                                const kio_vec_view* b,
                                kio_vec_view*       out);

KIO_API kio_status kio_vec_sub (const kio_vec_view* a,
                                const kio_vec_view* b,
                                kio_vec_view*       out);

KIO_API kio_status kio_vec_mul (const kio_vec_view* a,
                                const kio_vec_view* b,
                                kio_vec_view*       out);

KIO_API kio_status kio_vec_div (const kio_vec_view* a,
                                const kio_vec_view* b,
                                kio_vec_view*       out);

/* Dot product: *out = sum_i a[i] * b[i] */
KIO_API kio_status kio_vec_dot (const kio_vec_view* a,
                                const kio_vec_view* b,
                                double*             out);

/* Normalize: out[i] = a[i] / ||a|| */
KIO_API kio_status kio_vec_normalize(const kio_vec_view* a,
                                     kio_vec_view*       out);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KIO_C_CORE_ABI_H */

