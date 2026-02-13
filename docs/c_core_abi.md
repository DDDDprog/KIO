## KIO C Core ABI Design

This document specifies the **C-level ABI** for the future KIO C core. It is intentionally close to the existing C++ implementation, but:

- Removes dependence on C++ standard library types from the ABI surface.
- Makes value layouts and calling conventions explicit.
- Provides a clean boundary between:
  - **C core** (VM, bytecode, vector engine), and
  - **C++ host** (parser, JIT, LSP, GUI, modules, etc.).

The corresponding header is `c_core/abi.h`. This document explains the rationale behind it.

---

## Goals

- **Stable binary interface** for:
  - Executing KIO bytecode from C/C++ and other languages.
  - Calling native functions from KIO code (e.g., crypto, network, GPU).
  - Offloading vector math to a highly optimized C backend.
- **Portable C (C99)**:
  - No C++-only features in the ABI (no templates, exceptions, `std::vector`, `std::string`, etc.).
  - Compatible with shared-library use on Linux/macOS/Windows.
- **Bridgable** from the current C++ runtime:
  - The existing `kio::Value`, `Chunk`, and `VM` are easy to adapt to this ABI.
  - Native modules (e.g., `native_crypto_sha256`, `native_db_*`, `native_gpu_*`) can be migrated incrementally.

---

## Status / error model

### Status codes

The C core communicates success/failure via a small enum:

- `KIO_STATUS_OK` – operation succeeded.
- `KIO_STATUS_COMPILE_ERROR` – bytecode/chunk invalid or inconsistent.
- `KIO_STATUS_RUNTIME_ERROR` – runtime error (e.g., type error, out-of-bounds).
- `KIO_STATUS_OOM` – out of memory.
- `KIO_STATUS_INVALID_ARGUMENT` – invalid parameters at the ABI boundary.
- `KIO_STATUS_INTERNAL_ERROR` – unexpected internal failure.

These are used consistently across:

- VM creation / configuration.
- Chunk creation and emission (writing bytes, adding constants).
- VM execution.
- Vector operations.

### Extended error information

The `kio_error` struct gives optional rich errors:

- `status`: the high-level `kio_status`.
- `code`: domain-specific integer (e.g., VM error subtype).
- `message`: short, human-readable description (may be owned by the core or host; ownership rules are TBD but will be documented when implemented).

---

## Value representation

### C-level `kio_value`

The C value type is designed to match the **conceptual** shape of `kio::Value` in `include/kio/bytecode.hpp`, while being C-only:

- `kio_value_type`:
  - `KIO_VAL_NUMBER`
  - `KIO_VAL_BOOL`
  - `KIO_VAL_NIL`
  - `KIO_VAL_OBJ`
- `kio_value`:
  - `type` – a `kio_value_type`.
  - `as` – union of:
    - `double number`
    - `int32_t boolean` (0 or non-zero)
    - `kio_obj* obj` (opaque pointer to heap object).

Important notes:

- The **exact bit layout** of `kio_value` is defined by the header and must not change once the ABI is frozen.
- The internal `kio_obj` layout is **opaque** to ABI consumers:
  - C++ runtime can define `struct Obj` (strings, arrays, user objects).
  - C core functions that need to understand objects will either:
    - Be implemented in C++ wrappers, or
    - Receive additional, more specific types (e.g., vector views) instead of raw `kio_obj*`.

### Constructors and accessors

The ABI exposes helpers to construct and inspect values:

- Constructors:
  - `kio_make_nil()`
  - `kio_make_bool(int32_t b)`
  - `kio_make_number(double x)`
  - `kio_make_obj(kio_obj* o)`
- Predicates:
  - `kio_is_number`, `kio_is_bool`, `kio_is_nil`, `kio_is_obj`
- Accessors:
  - `kio_as_number`, `kio_as_bool`, `kio_as_obj`

These are simple inline-able helpers and provide a **canonical** way to construct values from C.

---

## Bytecode and chunks

### Opcodes

The C ABI defines `kio_opcode` as the VM’s instruction set. It is intentionally aligned with `OpCode` in `include/kio/bytecode.hpp`:

- Core opcodes:
  - Constants and literals: `KIO_OP_CONSTANT`, `KIO_OP_NIL`, `KIO_OP_TRUE`, `KIO_OP_FALSE`.
  - Stack ops: `KIO_OP_POP`, `KIO_OP_GET_LOCAL`, `KIO_OP_SET_LOCAL`, `KIO_OP_GET_GLOBAL`, `KIO_OP_DEFINE_GLOBAL`, `KIO_OP_SET_GLOBAL`.
  - Arithmetic and comparison: `KIO_OP_ADD`, `KIO_OP_SUBTRACT`, `KIO_OP_MULTIPLY`, `KIO_OP_DIVIDE`, `KIO_OP_MODULO`, `KIO_OP_EQUAL`, `KIO_OP_GREATER`, `KIO_OP_GREATER_EQUAL`, `KIO_OP_LESS`, `KIO_OP_LESS_EQUAL`.
  - Unary and truthiness: `KIO_OP_NOT`, `KIO_OP_NEGATE`.
  - Control flow: `KIO_OP_PRINT`, `KIO_OP_JUMP`, `KIO_OP_JUMP_IF_FALSE`, `KIO_OP_LOOP`.
  - Calls and builtins: `KIO_OP_CALL`.
  - Arrays: `KIO_OP_ARRAY_NEW`, `KIO_OP_ARRAY_GET`, `KIO_OP_ARRAY_SET`.
  - System queries: `KIO_OP_SYS_QUERY`.
  - Termination: `KIO_OP_HALT`.

This is the **canonical** instruction set for the C core; C++ code should map its `OpCode` values to these when emitting bytecode for the C VM.

### Chunks: `kio_chunk`

At the C ABI level, a chunk is an **opaque handle**:

- Creation / destruction:
  - `kio_chunk* kio_chunk_new(void);`
  - `void kio_chunk_free(kio_chunk* chunk);`
- Emission:
  - `kio_chunk_write(chunk, byte, line)` – append a single byte of code (with optional source line).
  - `kio_chunk_add_constant(chunk, value, &out_index)` – append a constant and get its index.

Internally, the C core is free to implement `kio_chunk` using:

- `std::vector` (on C++ side) in early implementations, or
- Raw `malloc`/`realloc`-backed arrays in a pure C implementation.

The ABI only promises:

- `code` is a byte array the VM can read sequentially.
- `constants` is an indexed table of `kio_value`.

---

## VM runtime ABI

### VM handle and lifecycle

The VM is represented as an opaque `kio_vm`:

- `kio_vm* kio_vm_new(void);`
- `void kio_vm_free(kio_vm* vm);`

This allows multiple independent VM instances in one process and keeps internal details (stack, frames, globals, JIT state) hidden from ABI consumers.

### Configuration

- `kio_vm_set_option(vm, key, value)`:
  - Generic key/value configuration mechanism (e.g. `"opt-level" = "3"`, `"jit" = "on"`, `"stack-size" = "8192"`).
  - Keys and values are UTF-8 strings; interpretation is implementation-defined but documented in future runtime docs.

### Execution

- `kio_status kio_vm_execute(kio_vm* vm, const kio_chunk* chunk, kio_value* out_result, kio_error* out_error);`
  - Executes bytecode in `chunk` starting at IP = 0.
  - On success:
    - Returns `KIO_STATUS_OK`.
    - If `out_result` is non-null, stores the last value on the stack or a canonical `nil`.
  - On failure:
    - Returns a non-OK `kio_status`.
    - If `out_error` is non-null, fills it with additional context (short message, subtype code).

Internally, this corresponds to what `kio::VM::interpret(Chunk*)` and `VM::run()` do today with:

- Fixed-size stack (`STACK_MAX`).
- `CallFrame` array (`FRAMES_MAX`).
- `globals_` and builtin functions.
- Hot-loop JIT dispatch for `LOOP` opcodes.

---

## Native function ABI

Many existing modules (`http_server`, `gpu_bridge`, `gmp_bridge`, `vector_ops`, `gui_bridge`, etc.) already use a de facto ABI:

```cpp
Value native_fn(int argCount, Value* args);
```

The C ABI formalizes and extends this into:

```c
typedef enum kio_native_status {
    KIO_NATIVE_OK = 0,
    KIO_NATIVE_ERROR = 1
} kio_native_status;

typedef struct kio_native_ctx kio_native_ctx; /* Opaque */

typedef kio_native_status (*kio_native_fn)(
    int             arg_count,
    const kio_value* args,
    kio_value*      out_result,
    kio_native_ctx* ctx);
```

Key points:

- The function receives:
  - `arg_count` and `args` as a read-only array of `kio_value`.
  - A writable `out_result` location.
  - An opaque `kio_native_ctx` that can expose VM-specific services in future (e.g. allocation, module lookup).
- It returns `KIO_NATIVE_OK` on success, `KIO_NATIVE_ERROR` on failure.
- Failure details can be further surfaced to the VM via internal mechanisms (e.g. attaching to `kio_error`).

To attach a native function to a KIO VM:

- `kio_vm_register_native(vm, "crypto.sha256", sha256_fn, user_data);`

This mirrors the existing pattern where functions like `native_crypto_sha256`, `native_server_*`, `native_gpu_*`, and `native_bigint_*` are installed into the runtime’s global namespace or module system.

---

## Vector engine ABI

The vector engine is explicitly designed for **numeric inner loops** and is not constrained to the dynamic `kio_value` representation. Instead, it operates on **dense double buffers**:

### Vector view

```c
typedef struct kio_vec_view {
    double* data;
    size_t  length;
    size_t  stride; /* in elements; usually 1 */
} kio_vec_view;
```

- `data`: pointer to the first element.
- `length`: number of elements in the view.
- `stride`: step in elements between consecutive entries (allows basic SoA / subviews).

On the C++ side, this maps naturally from `ObjArray` in `include/kio/bytecode.hpp`:

- Extract `double` values from `ObjArray::elements` into a contiguous buffer (or validate they already are numeric and tightly packed).
- Wrap them in `kio_vec_view`.

### Operations

The ABI exposes the following primitives:

- Elementwise operations (in-place into `out`):
  - `kio_vec_add(a, b, out)` – `out[i] = a[i] + b[i]`
  - `kio_vec_sub(a, b, out)` – `out[i] = a[i] - b[i]`
  - `kio_vec_mul(a, b, out)` – `out[i] = a[i] * b[i]`
  - `kio_vec_div(a, b, out)` – `out[i] = a[i] / b[i]`
- Reductions:
  - `kio_vec_dot(a, b, &out)` – returns the dot product in `out`.
- Normalization:
  - `kio_vec_normalize(a, out)` – `out[i] = a[i] / ||a||`.

All operations:

- Validate lengths and strides (`KIO_STATUS_INVALID_ARGUMENT` on mismatch).
- Are free to exploit SIMD (SSE/AVX/NEON) and other micro-optimizations internally.
- Return `KIO_STATUS_OK` on success or `KIO_STATUS_RUNTIME_ERROR` (e.g. NaNs, division by zero) where appropriate.

This is the C-level replacement for the current C++ functions in `src/vector/vector_ops.cpp` (`vectorized_add`, `vectorized_sub`, etc.).

---

## Integration with existing C++ runtime

### Mapping of types

- `kio::Value` ↔ `kio_value`:
  - Short term: conversion helpers in a C++ bridge layer (allocate `kio_value` from `kio::Value` and vice versa).
  - Long term: align internal layouts so that a `reinterpret_cast` is valid, guarded by static assertions.
- `kio::Chunk` ↔ `kio_chunk`:
  - C++ compiler (`src/vm/compiler.cpp`) can either:
    - Continue to emit into its own `Chunk` and then copy into a `kio_chunk`, or
    - Be refactored to emit directly into the C chunk API.
- `kio::VM` ↔ `kio_vm`:
  - Initially, the C VM can wrap the existing C++ `VM` (delegating `kio_vm_execute` to `VM::interpret`).
  - Later, the hot execution path (`VM::run`) can be reimplemented in C while keeping the C++ facade for compatibility.

### Native modules

Existing native functions already fit the ABI shape conceptually:

- They currently take `(int argCount, Value* args)` and return `Value`.
- Under the C ABI, they should be migrated to `kio_native_fn`:
  - Accept `const kio_value*` and `kio_native_ctx*`.
  - Write to an explicit `kio_value* out_result`.
  - Return a `kio_native_status`.

This migration can be done module-by-module (crypto, network, database, gpu, gui, misc, vector).

---

## Summary

- The **C core ABI** is defined in `c_core/abi.h` and is intentionally:
  - Minimal, C-only, and stable.
  - Close to today’s `bytecode.hpp`/`vm.hpp` semantics.
  - Ready to host a high-performance C implementation of:
    - The VM execution loop.
    - The value representation and stack frames.
    - The vector math engine.
- The C++ codebase will gradually:
  - Emit bytecode via `kio_chunk`.
  - Execute programs via `kio_vm_execute`.
  - Register and call native modules via `kio_vm_register_native` and `kio_native_fn`.

This provides the foundation needed for the future **C core** while keeping the current C++ implementation usable and debuggable during the transition.

