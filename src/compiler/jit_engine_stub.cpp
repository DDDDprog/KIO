/*
 * Stub JIT engine implementation used when KIO_ENABLE_JIT=OFF.
 *
 * This provides no-op definitions for the JITEngine class so that the VM
 * can link successfully without pulling in LLVM or any JIT infrastructure.
 */

#include "axeon/jit_engine.hpp"

namespace kio {

struct JITEngine::Impl {};

JITEngine::JITEngine() = default;
JITEngine::~JITEngine() = default;

JITEngine::CompiledLoop JITEngine::compileLoop(Chunk*, uint8_t*) {
    // JIT is disabled; signal no compiled loop is available.
    return nullptr;
}

} // namespace kio

