/*
 * AXEON C Core ABI – C++ bridge implementation
 *
 * This file provides an initial implementation of the C-level ABI declared in
 * c_core/abi.h by forwarding to the existing C++ VM and bytecode structures.
 *
 * It allows embedders to:
 *  - Construct chunks and append bytecode / constants.
 *  - Create a VM instance and execute a chunk.
 *  - Use a simple C-based vector engine over double buffers.
 *
 * NOTE: Native function registration is currently a stub and will be wired
 *       into the builtin/module system in a later iteration.
 */

#include "abi.h"

#include "axeon/bytecode.hpp"
#include "axeon/vm.hpp"

#include <cmath>
#include <new>

// Concrete definitions for opaque C types declared in abi.h.
// These wrap the existing C++ runtime structures.
struct kio_chunk {
    kio::Chunk chunk;
};

struct kio_vm {
    kio::VM vm;
};

// kio_obj and kio_native_ctx remain opaque; object interop is not yet wired.

namespace {

using namespace kio;

// --- Value conversion helpers ----------------------------------------------

static kio_value to_c_value(const Value& v) {
    kio_value out{};
    switch (v.type) {
        case ValueType::VAL_NIL:
            out.type = AXEON_VAL_NIL;
            out.as.number = 0.0;
            out.as.boolean = 0;
            out.as.obj = nullptr;
            break;
        case ValueType::VAL_BOOL:
            out.type = AXEON_VAL_BOOL;
            out.as.boolean = v.as.boolean ? 1 : 0;
            out.as.number = 0.0;
            out.as.obj = nullptr;
            break;
        case ValueType::VAL_NUMBER:
            out.type = AXEON_VAL_NUMBER;
            out.as.number = v.as.number;
            out.as.boolean = 0;
            out.as.obj = nullptr;
            break;
        case ValueType::VAL_OBJ:
            // Object interop is not yet surfaced through the C ABI.
            out.type = AXEON_VAL_OBJ;
            out.as.obj = nullptr;
            out.as.number = 0.0;
            out.as.boolean = 0;
            break;
    }
    return out;
}

static Value to_cpp_value(const kio_value& v) {
    switch (v.type) {
        case AXEON_VAL_NUMBER:
            return doubleToValue(v.as.number);
        case AXEON_VAL_BOOL:
            return BOOL_VAL(v.as.boolean != 0);
        case AXEON_VAL_NIL:
            return NIL_VAL;
        case AXEON_VAL_OBJ:
        default:
            // Object values are not yet supported through the C ABI bridge.
            return NIL_VAL;
    }
}

static kio_status from_interpret_result(InterpretResult r) {
    switch (r) {
        case InterpretResult::OK:            return AXEON_STATUS_OK;
        case InterpretResult::COMPILE_ERROR: return AXEON_STATUS_COMPILE_ERROR;
        case InterpretResult::RUNTIME_ERROR: return AXEON_STATUS_RUNTIME_ERROR;
    }
    return AXEON_STATUS_INTERNAL_ERROR;
}

// --- Vector helper ---------------------------------------------------------

static kio_status check_vec_compatible(const kio_vec_view* a,
                                       const kio_vec_view* b,
                                       const kio_vec_view* out) {
    if (!a || !b || !out) return AXEON_STATUS_INVALID_ARGUMENT;
    if (!a->data || !b->data || !out->data) return AXEON_STATUS_INVALID_ARGUMENT;
    if (a->stride == 0 || b->stride == 0 || out->stride == 0)
        return AXEON_STATUS_INVALID_ARGUMENT;

    // Use the minimum logical length across inputs.
    size_t len = a->length;
    if (b->length < len) len = b->length;
    if (out->length < len) len = out->length;
    return (len == 0) ? AXEON_STATUS_INVALID_ARGUMENT : AXEON_STATUS_OK;
}

} // namespace

// ---------------------------------------------------------------------------
// Value constructors / predicates
// ---------------------------------------------------------------------------

kio_value kio_make_nil(void) {
    kio_value v{};
    v.type = AXEON_VAL_NIL;
    v.as.number = 0.0;
    v.as.boolean = 0;
    v.as.obj = nullptr;
    return v;
}

kio_value kio_make_bool(int32_t b) {
    kio_value v{};
    v.type = AXEON_VAL_BOOL;
    v.as.boolean = b ? 1 : 0;
    v.as.number = 0.0;
    v.as.obj = nullptr;
    return v;
}

kio_value kio_make_number(double x) {
    kio_value v{};
    v.type = AXEON_VAL_NUMBER;
    v.as.number = x;
    v.as.boolean = 0;
    v.as.obj = nullptr;
    return v;
}

kio_value kio_make_obj(kio_obj* o) {
    kio_value v{};
    v.type = AXEON_VAL_OBJ;
    v.as.obj = o;
    v.as.number = 0.0;
    v.as.boolean = 0;
    return v;
}

int kio_is_number(kio_value v) { return v.type == AXEON_VAL_NUMBER; }
int kio_is_bool  (kio_value v) { return v.type == AXEON_VAL_BOOL; }
int kio_is_nil   (kio_value v) { return v.type == AXEON_VAL_NIL; }
int kio_is_obj   (kio_value v) { return v.type == AXEON_VAL_OBJ; }

double   kio_as_number(kio_value v) { return v.as.number; }
int32_t  kio_as_bool  (kio_value v) { return v.as.boolean != 0; }
kio_obj* kio_as_obj   (kio_value v) { return v.as.obj; }

// ---------------------------------------------------------------------------
// Chunk implementation
// ---------------------------------------------------------------------------

kio_chunk* kio_chunk_new(void) {
    return new (std::nothrow) kio_chunk{};
}

void kio_chunk_free(kio_chunk* chunk) {
    delete chunk;
}

kio_status kio_chunk_write(kio_chunk* chunk, uint8_t byte, int line) {
    if (!chunk) return AXEON_STATUS_INVALID_ARGUMENT;
    chunk->chunk.write(byte, line);
    return AXEON_STATUS_OK;
}

kio_status kio_chunk_add_constant(kio_chunk* chunk,
                                  kio_value   value,
                                  int*        out_index) {
    if (!chunk || !out_index) return AXEON_STATUS_INVALID_ARGUMENT;
    Value v = to_cpp_value(value);
    int idx = chunk->chunk.addConstant(v);
    *out_index = idx;
    return AXEON_STATUS_OK;
}

// ---------------------------------------------------------------------------
// VM implementation
// ---------------------------------------------------------------------------

kio_vm* kio_vm_new(void) {
    return new (std::nothrow) kio_vm{};
}

void kio_vm_free(kio_vm* vm) {
    delete vm;
}

kio_status kio_vm_set_option(kio_vm* vm, const char* key, const char* value) {
    (void)vm;
    (void)key;
    (void)value;
    // Options are not yet wired; accept and ignore for now.
    return AXEON_STATUS_OK;
}

kio_status kio_vm_execute(kio_vm*          vm,
                          const kio_chunk* chunk,
                          kio_value*       out_result,
                          kio_error*       out_error) {
    if (!vm || !chunk) return AXEON_STATUS_INVALID_ARGUMENT;

    InterpretResult r = vm->vm.interpret(const_cast<Chunk*>(&chunk->chunk));
    kio_status status = from_interpret_result(r);

    if (out_result) {
        // The current VM does not expose a "last value" result; return nil.
        *out_result = kio_make_nil();
    }

    if (out_error) {
        out_error->status = status;
        out_error->code = 0;
        out_error->message = nullptr;
    }

    return status;
}

// ---------------------------------------------------------------------------
// Native function registration (stub)
// ---------------------------------------------------------------------------

kio_status kio_vm_register_native(kio_vm*       vm,
                                  const char*   name,
                                  kio_native_fn fn,
                                  void*         user_data) {
    (void)vm;
    (void)name;
    (void)fn;
    (void)user_data;
    // TODO: Wire this into BuiltinFunctions / ModuleSystem so that C-ABI
    // native functions can be called from AXEON code.
    return AXEON_STATUS_INTERNAL_ERROR;
}

// ---------------------------------------------------------------------------
// Vector engine implementation
// ---------------------------------------------------------------------------

kio_status kio_vec_add(const kio_vec_view* a,
                       const kio_vec_view* b,
                       kio_vec_view*       out) {
    kio_status st = check_vec_compatible(a, b, out);
    if (st != AXEON_STATUS_OK) return st;

    size_t len = a->length;
    if (b->length < len) len = b->length;
    if (out->length < len) len = out->length;

    for (size_t i = 0; i < len; ++i) {
        size_t ia = i * a->stride;
        size_t ib = i * b->stride;
        size_t io = i * out->stride;
        out->data[io] = a->data[ia] + b->data[ib];
    }
    return AXEON_STATUS_OK;
}

kio_status kio_vec_sub(const kio_vec_view* a,
                       const kio_vec_view* b,
                       kio_vec_view*       out) {
    kio_status st = check_vec_compatible(a, b, out);
    if (st != AXEON_STATUS_OK) return st;

    size_t len = a->length;
    if (b->length < len) len = b->length;
    if (out->length < len) len = out->length;

    for (size_t i = 0; i < len; ++i) {
        size_t ia = i * a->stride;
        size_t ib = i * b->stride;
        size_t io = i * out->stride;
        out->data[io] = a->data[ia] - b->data[ib];
    }
    return AXEON_STATUS_OK;
}

kio_status kio_vec_mul(const kio_vec_view* a,
                       const kio_vec_view* b,
                       kio_vec_view*       out) {
    kio_status st = check_vec_compatible(a, b, out);
    if (st != AXEON_STATUS_OK) return st;

    size_t len = a->length;
    if (b->length < len) len = b->length;
    if (out->length < len) len = out->length;

    for (size_t i = 0; i < len; ++i) {
        size_t ia = i * a->stride;
        size_t ib = i * b->stride;
        size_t io = i * out->stride;
        out->data[io] = a->data[ia] * b->data[ib];
    }
    return AXEON_STATUS_OK;
}

kio_status kio_vec_div(const kio_vec_view* a,
                       const kio_vec_view* b,
                       kio_vec_view*       out) {
    kio_status st = check_vec_compatible(a, b, out);
    if (st != AXEON_STATUS_OK) return st;

    size_t len = a->length;
    if (b->length < len) len = b->length;
    if (out->length < len) len = out->length;

    for (size_t i = 0; i < len; ++i) {
        size_t ia = i * a->stride;
        size_t ib = i * b->stride;
        size_t io = i * out->stride;
        double denom = b->data[ib];
        out->data[io] = (denom != 0.0) ? (a->data[ia] / denom) : 0.0;
    }
    return AXEON_STATUS_OK;
}

kio_status kio_vec_dot(const kio_vec_view* a,
                       const kio_vec_view* b,
                       double*             out) {
    if (!out) return AXEON_STATUS_INVALID_ARGUMENT;
    if (!a || !b || !a->data || !b->data) return AXEON_STATUS_INVALID_ARGUMENT;
    if (a->stride == 0 || b->stride == 0) return AXEON_STATUS_INVALID_ARGUMENT;

    size_t len = a->length;
    if (b->length < len) len = b->length;
    if (len == 0) return AXEON_STATUS_INVALID_ARGUMENT;

    double acc = 0.0;
    for (size_t i = 0; i < len; ++i) {
        size_t ia = i * a->stride;
        size_t ib = i * b->stride;
        acc += a->data[ia] * b->data[ib];
    }
    *out = acc;
    return AXEON_STATUS_OK;
}

kio_status kio_vec_normalize(const kio_vec_view* a,
                             kio_vec_view*       out) {
    if (!a || !out || !a->data || !out->data) return AXEON_STATUS_INVALID_ARGUMENT;
    if (a->stride == 0 || out->stride == 0) return AXEON_STATUS_INVALID_ARGUMENT;

    size_t len = a->length;
    if (out->length < len) len = out->length;
    if (len == 0) return AXEON_STATUS_INVALID_ARGUMENT;

    double mag_sq = 0.0;
    for (size_t i = 0; i < len; ++i) {
        size_t ia = i * a->stride;
        double v = a->data[ia];
        mag_sq += v * v;
    }
    if (mag_sq == 0.0) return AXEON_STATUS_RUNTIME_ERROR;

    double mag = std::sqrt(mag_sq);
    for (size_t i = 0; i < len; ++i) {
        size_t ia = i * a->stride;
        size_t io = i * out->stride;
        out->data[io] = a->data[ia] / mag;
    }
    return AXEON_STATUS_OK;
}

