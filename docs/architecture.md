## KIO Architecture Overview

This document describes the current KIO architecture as implemented in the C++ codebase, focusing on the VM, parser, LSP, GUI bridge, vector operations, and related modules. It is a snapshot of the **2026** implementation and is intended to guide the transition to a future C core.

---

## High-level pipeline

- **Frontend**
  - `Lexer` (`src/tokenizer/lexer.cpp`): Converts source text into a stream of `Token`s.
  - `Parser` (`src/parser/parser.cpp` + `include/kio/ast.hpp`): Builds an AST (`Expr`/`Stmt` variants) from tokens.
  - Optional **AST visitors** (`src/parser/ast_visitor.cpp`) traverse the tree for analysis or transformations.
- **Execution paths**
  - **Bytecode VM path (primary)**:
    - `Compiler` (`src/vm/compiler.cpp`) walks the AST and emits bytecode into a `Chunk` (`include/kio/bytecode.hpp`).
    - `VM` (`src/vm/vm.cpp`, `include/kio/vm.hpp`) executes the bytecode on a stack-based virtual machine.
  - **Tree-walking interpreter (secondary/legacy)**:
    - `Interpreter` (`src/core/interpreter.cpp`) directly evaluates the AST without bytecode.
- **Tooling / integration**
  - **LSP server** (`src/lsp/*.cpp`) provides completions, diagnostics, hover, go-to-definition, etc.
  - **GUI / GPU / Network / Crypto / Database / Misc** modules expose native functionality as **builtin functions** callable from KIO.

Entry point: `src/core/main.cpp` wires up `Lexer` → `Parser` → `Compiler` → `VM` and provides the CLI / REPL.

---

## Module layout

- **Core**
  - `src/core/main.cpp`: CLI / REPL, file loading, error reporting.
  - `src/core/value.cpp` + `include/kio/bytecode.hpp`: `Value`, `ObjString`, `ObjArray` and helpers (`valueToDouble`, `objToValue`, etc.).
  - `src/core/module_system.cpp`: In-process module registry for functions/variables (`Module`, `ModuleSystem`).
  - `src/core/config.cpp`: Configuration and environment-driven options.
  - `src/core/interpreter.cpp`: Tree-walking interpreter.
- **VM / bytecode**
  - `include/kio/bytecode.hpp`: Defines `OpCode`, `ValueType`, `Value`, object model, and `Chunk`.
  - `src/vm/opcode.hpp`: Alternate enum of VM opcodes used by the compiler.
  - `src/vm/chunk.hpp`: Earlier `Chunk`/`Value` representation (std::variant-based), now largely superseded by `bytecode.hpp`.
  - `src/vm/compiler.cpp`: AST → bytecode compiler, manages locals, scopes, jumps, loops, array ops, `sys` queries, etc.
  - `src/vm/vm.cpp`: Core bytecode interpreter (`VM::run`), global environment, and hot-loop JIT integration.
- **Frontend (lexing/parsing)**
  - `src/tokenizer/lexer.cpp`: Tokenization, keyword recognition, numeric/string/char literals, comments, and configurable aliases.
  - `src/tokenizer/token_stream.cpp`, `src/tokenizer/preprocessor.cpp`: Additional token stream / preprocessing utilities.
  - `include/kio/ast.hpp`: AST definitions for expressions and statements.
  - `src/parser/parser.cpp`: Recursive-descent parser for declarations, statements, and expressions.
  - `src/parser/parser_extensions.cpp`, `src/parser/error_recovery.cpp`: Extended constructs and error handling.
  - `src/parser/ast_visitor.cpp`: Generic AST visitor traversing all `Expr`/`Stmt` variants.
- **LSP**
  - `src/lsp/main.cpp`: LSP server entry point.
  - `src/lsp/lsp_server.cpp`: Core protocol handling.
  - `src/lsp/document_manager.cpp`: Tracks open documents and their parsed state.
  - `src/lsp/completion.cpp`: Completions based on cursor context, builtins, and keywords.
  - `src/lsp/diagnostics.cpp`, `src/lsp/formatting.cpp`, `src/lsp/goto_definition.cpp`, `src/lsp/hover.cpp`, `src/lsp/semantic_tokens.cpp`: IDE features built on top of the parser and symbol model.
- **Native feature modules**
  - `src/vector/vector_ops.cpp`: AVX2-accelerated vector math on `ObjArray` (`vectorized_add/sub/mul/div/dot/cross/normalize`).
  - `src/network/http_server.cpp`: Blocking TCP HTTP server as native functions (`native_server_*`), returning `ObjArray` payloads.
  - `src/crypto/sha256.cpp`: Native hashing (`native_crypto_sha256`) returning hex strings (currently backed by `std::hash` placeholder).
  - `src/database/db_driver.cpp`: In-memory key/value database (`KioDB`) with `native_db_get/set`.
  - `src/gpu/gpu_bridge.cpp`: GPU integration stubs (`native_gpu_*`) for ROCm/rocSHMEM-style backends.
  - `src/gui/gui_bridge.cpp`: Simple console-backed GUI stubs (`native_gui_window`, `native_gui_button`).
  - `src/misc/gmp_bridge.cpp`: GMP-backed big integer operations exported as native functions.
- **JIT and advanced features**
  - `src/features/jit_engine.cpp`, `src/features/jit_compiler.cpp`, `src/features/fast_jit.cpp`, `src/features/tracing_jit*.cpp`: JIT compilation for hot loops (`VM::LOOP` opcode) and tracing.
  - `src/features/optimizer.cpp`, `src/features/type_system.cpp`, `src/features/memory_manager.cpp`, `src/features/parallel_executor.cpp`: Optimizations, type analysis, memory, and parallel execution infrastructure.

---

## Current performance hotspots (by design/implementation)

### 1. VM execution loop

- **File(s)**: `src/vm/vm.cpp`, `include/kio/vm.hpp`, `include/kio/bytecode.hpp`
- **Hot code paths**:
  - `VM::run()` main dispatch loop:
    - Bytecode fetch/decode: `OpCode inst = (OpCode)(*ip++);`
    - Stack operations on `Value stack_[STACK_MAX]` and `sp`.
    - Arithmetic/logical opcodes (`ADD`, `SUBTRACT`, `MULTIPLY`, `DIVIDE`, `MODULO`, comparison ops).
    - Control flow opcodes (`JUMP`, `JUMP_IF_FALSE`, `LOOP`) and hot-loop detection via `loop_hits_`.
  - Builtin dispatch inside `OpCode::CALL`:
    - Resolves callee name via `ObjString` and looks up in `BuiltinFunctions`.
  - Array opcodes (`ARRAY_NEW`, `ARRAY_GET`, `ARRAY_SET`) operating over `ObjArray`.
- **Why it’s hot**:
  - Every non-trivial program spends most of its time in `VM::run`.
  - Tight inner loops in user code translate to repeated `LOOP` opcodes, which also feed the JIT (`jit_.compileLoop`).
  - Heavy use of dynamic `Value` representation and heap-allocated `ObjString` / `ObjArray` objects.

### 2. Compiler and bytecode generation

- **File(s)**: `src/vm/compiler.cpp`, `include/kio/ast.hpp`
- **Hot code paths**:
  - `Compiler::compile(const std::vector<StmtPtr>&)` iterates over all top-level statements and emits opcodes into a `Chunk`.
  - `Compiler::compileExpr` recursively visits expression trees and maps `TokenType` to `OpCode` (e.g. arithmetic, comparison, logical).
  - Scope management: `addLocal`, `resolveLocal`, and scope-depth-based local cleanup in block/loop compilation.
  - Jump/loop patching: `emitJump`, `patchJump`, `emitLoop`.
- **Why it’s hot**:
  - Re-run on every compilation (REPL inputs, file loads).
  - Complexity grows with AST size; deeply nested expressions and control flow structures generate significant bytecode.

### 3. Lexer and parser

- **File(s)**: `src/tokenizer/lexer.cpp`, `src/parser/parser.cpp`, `include/kio/ast.hpp`
- **Hot code paths**:
  - `Lexer::scanTokens` main loop and helpers:
    - `identifier`, `number`, `stringLiteral`, `charLiteral`.
    - Keyword table lookup and alias expansion (`Config::fromEnv()`).
  - `Parser::parse` and recursive-descent methods:
    - Declaration-level: `declaration`, `functionDeclaration`, `varDeclaration`, `constDeclaration`, `moduleDeclaration`, `importStatement`, etc.
    - Statement-level: `ifStatement`, `whileStatement`, `forStatement` / `ForIn`, `block`, `parallelStatement`, etc.
    - Expression-level: `assignment`, `or_expr`, `and_expr`, `equality`, `comparison`, `term`, `factor`, `unary`, `call`, `primary`, `array`, `ternary`, `sysQuery`.
- **Why it’s hot**:
  - Every compilation run starts by lexing and parsing.
  - String-heavy operations (substr, identifier/keyword lookup).
  - Error-paths can be costly if the code is syntactically incorrect (exception creation and stack unwinding).

### 4. Vector operations

- **File(s)**: `src/vector/vector_ops.cpp`
- **Hot code paths**:
  - SIMD-enabled loops in:
    - `vectorized_add`, `vectorized_sub`, `vectorized_mul`, `vectorized_div`.
    - `vectorized_dot`, `vectorized_normalize`, `vectorized_cross`.
  - Conversions between `Value` and `double` via `valueToDouble` / `doubleToValue`.
- **Why it’s hot**:
  - Designed explicitly for numerical inner loops over large `ObjArray` buffers.
  - Uses AVX2 intrinsics when available, falling back to scalar loops.
  - Performance is sensitive to:
    - Memory layout of `ObjArray::elements`.
    - Branch checks (`isNumber`) and bounds.

### 5. Native modules and IO

- **Network / HTTP**
  - **File(s)**: `src/network/http_server.cpp`
  - Hot paths: `native_server_init`, `native_server_accept`, `native_server_respond`.
  - Workload: blocking `accept`, `read`, request parsing, and `send` of full HTTP responses.
- **Crypto / hashing**
  - **File(s)**: `src/crypto/sha256.cpp`
  - Hot paths: `native_crypto_sha256` (currently using `std::hash` as a placeholder; real crypto would be significantly heavier).
- **Database / storage**
  - **File(s)**: `src/database/db_driver.cpp`
  - In-memory `KioDB` (`std::unordered_map<std::string,std::string>`) plus simple persistence hooks.
- **Big integer / GMP**
  - **File(s)**: `src/misc/gmp_bridge.cpp`
  - Hot paths: `native_bigint_*` operations, including `mpz_add`, `mpz_mul`, etc., plus string conversions.
- **GPU bridge**
  - **File(s)**: `src/gpu/gpu_bridge.cpp`
  - Currently mostly stubs; will become a hotspot once real ROCm / HIP calls are wired.

These modules are invoked from KIO via **builtin functions** bound to names in the global environment (through `BuiltinFunctions` and, in future, the module system).

### 6. LSP and tooling

- **File(s)**: `src/lsp/*.cpp`
- **Potential hotspots**:
  - Repeated lex/parse cycles per edit for diagnostics, semantic tokens, and completion.
  - `CompletionProvider::get_completions` in `src/lsp/completion.cpp` when used on large documents.
  - Symbol resolution, go-to-definition, and semantic classification.
- **Why it matters**:
  - LSP responsiveness is critical for perceived IDE performance.
  - Shares frontend costs (lexer/parser) with the main compiler/VM pipeline.

---

## Summary for C-core migration

- The **VM stack loop**, **vector operations**, and **lexer/parser** form the primary **performance hotspots** today.
- The architecture is already modular:
  - Frontend (tokenizer/parser/AST),
  - Execution engines (VM, interpreter, JIT),
  - Native modules (vector, crypto, data/database, gpu, gui, misc, network),
  - Tooling (LSP).
- A C-based core can initially target:
  - The `Value` representation and stack frame layout.
  - The VM dispatch loop and arithmetic/array opcodes.
  - The vector math engine over `ObjArray`-like buffers.
- Higher-level C++ layers (parser, LSP, module system, JIT) can remain largely unchanged and call into the C core via a stable C ABI.

