## KIO Module Layout and Responsibilities

This document describes the **top-level runtime modules** in the current KIO implementation. These modules will be the primary integration points for the future C core.

---

## Core runtime and VM

- **Core**
  - `src/core/`:
    - `main.cpp`: CLI / REPL and program entry.
    - `value.cpp`: dynamic `Value` implementation built on `include/kio/bytecode.hpp`.
    - `module_system.cpp`: in-process module registry (functions and variables).
    - `config.cpp`: configuration and keyword aliasing.
    - `interpreter.cpp`: tree-walking interpreter.
- **VM**
  - `src/vm/` + `include/kio/vm.hpp` + `include/kio/bytecode.hpp`:
    - Bytecode VM (`VM`), compiler (`Compiler`), opcode set (`OpCode`), and `Chunk`.
    - This is the main target for the future C core VM.

---

## Domain-specific modules

### Crypto (`src/crypto/`)

- **Files**: `src/crypto/sha256.cpp`, `src/crypto/aes.cpp`
- **Responsibilities**:
  - Provide hashing and (eventually) symmetric encryption primitives to KIO code.
  - Expose functions like `crypto.sha256(data: string) -> string`.
  - Integrate with standard library-style modules (e.g. `import "crypto"`).
- **Initial C-core interface**:
  - Implemented as **native functions** on top of `kio_native_fn`, reusing the C value ABI.
  - Future work may move hot paths (real SHA-256/AES implementations) into C for maximum portability and performance.

### Data and Database (`src/data/`, `src/database/`)

- **Files**: `src/data/token.cpp`, `src/database/db_driver.cpp`
- **Responsibilities**:
  - `src/data/`: Core data structures used by the language implementation (e.g., token metadata) rather than user-facing APIs.
  - `src/database/`: Embedded key/value storage for user programs:
    - `KioDB` using `std::unordered_map<std::string, std::string>`.
    - Native functions `native_db_set` / `native_db_get`.
- **Initial C-core interface**:
  - C core calls into database primitives via the native function ABI.
  - Future: introduce a stable, user-facing `database` module in KIO with a minimal API (set/get, persistence hooks, iteration).

### GPU (`src/gpu/`)

- **Files**: `src/gpu/gpu_bridge.cpp`
- **Responsibilities**:
  - Act as a thin bridge to GPU/accelerator backends (ROCm / rocSHMEM, HIP, CUDA, etc.).
  - Provide native functions like:
    - `gpu.init()`
    - `gpu.malloc(size: number) -> handle`
    - `gpu.sync()`
- **Initial C-core interface**:
  - GPU ops remain native functions bound into the VM.
  - When the C core gains its own scheduler or task abstraction, GPU offload can be modeled as async jobs associated with the VM.

### GUI (`src/gui/`)

- **Files**: `src/gui/gui_bridge.cpp`
- **Responsibilities**:
  - Provide a minimal, platform-agnostic GUI API (currently console-backed stubs):
    - `gui.window(title: string, width: number, height: number) -> bool`
    - `gui.button(text: string) -> nil`
  - Long-term: back these functions with a real GUI toolkit while keeping the KIO API stable.
- **Initial C-core interface**:
  - GUI calls remain as native functions that operate on `kio_value` arguments.
  - The C core itself remains GUI-agnostic; it only needs to support the native function ABI.

### Misc / BigInt (`src/misc/`)

- **Files**: `src/misc/gmp_bridge.cpp`
- **Responsibilities**:
  - Provide big integer arithmetic via GMP:
    - `bigint.add/sub/mul/div/mod/pow/cmp` on decimal string operands.
  - Serve as the foundation for any high-precision numeric types in the standard library.
- **Initial C-core interface**:
  - BigInt operations are exposed as native functions.
  - C core knows nothing about GMP directly; it just passes `kio_value` arguments and receives results.

### Network (`src/network/`)

- **Files**: `src/network/http_server.cpp`
- **Responsibilities**:
  - Implement a blocking HTTP server API:
    - `net.server_init(port: number) -> id`
    - `net.server_accept(id) -> [fd, method, path, raw_request]`
    - `net.server_respond(fd, status, content_type, body) -> bool`
  - Provide building blocks for higher-level HTTP libraries in KIO.
- **Initial C-core interface**:
  - Network APIs are native functions that operate on `kio_value` lists/arrays and strings.
  - C core is only responsible for calling them and handling errors via `kio_native_status`.

### Vector (`src/vector/`)

- **Files**: `src/vector/vector_ops.cpp`
- **Responsibilities**:
  - High-performance vector math on KIO arrays (`ObjArray`), including:
    - `vector.add/sub/mul/div`
    - `vector.dot`, `vector.cross`, `vector.normalize`
  - Use AVX2 (and other SIMD backends) when available for numerical hot loops.
- **Initial C-core interface**:
  - Will be refactored to use the C-level **vector engine ABI**:
    - Wrap `ObjArray` buffers as `kio_vec_view`.
    - Call `kio_vec_add/sub/mul/div/dot/normalize` for the heavy lifting.

---

## Relationship to the future C core

- The existing C++ modules already align well with the planned **C core + C++ host** split:
  - **C core**:
    - VM execution loop and value representation (`c_core/`).
    - Vector engine (`kio_vec_*`).
  - **C++ host**:
    - Parser, lexer, AST, JIT.
    - Native modules in `src/crypto`, `src/database`, `src/network`, `src/gpu`, `src/gui`, `src/misc`, `src/vector`.
- As the C core matures:
  - New C implementations can replace the hottest native functions (e.g., SHA-256, AES, vector math) with C libraries while preserving the same KIO-level API.
  - Direct C bindings (via `kio_native_fn`) will make it easier to expose low-level hardware features without impacting the language front-end.

