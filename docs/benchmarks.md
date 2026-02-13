## KIO Benchmark and Testing Strategy

This document describes how we will **measure and compare performance** between:

- The existing C++ VM/runtime, and
- The new C core VM and vector engine defined in `c_core/abi.h`.

The initial focus is on **repeatable microbenchmarks** and a small set of **macrobenchmarks** that stress realistic workloads.

---

## Objectives

- Track performance of:
  - VM bytecode execution (arithmetic, control flow, function calls).
  - Vector and numeric-heavy workloads.
  - Native-module boundaries (crypto, database, network, GPU, BigInt).
- Provide **A/B comparisons**:
  - `engine = cpp` (current C++ VM).
  - `engine = c_core` (future C core VM).
- Integrate with existing tooling where possible:
  - CTest, `kio_build.py`, and standard UNIX tools (`time`, `perf`, `valgrind`).

---

## Benchmark categories

### 1. VM microbenchmarks

Small KIO programs that stress specific bytecode patterns.

- **Arithmetic loop**

```kio
let sum = 0;
for (let i = 0; i < 10_000_000; i = i + 1) {
    sum = sum + i;
}
print sum;
```

Metrics:

- Wall-clock time.
- Instructions per second (from `perf` on Linux).

Variants:

- Integer-like arithmetic (within safe `double` range).
- Mixed arithmetic and branching.

---

- **Function call overhead**

```kio
function inc(x: number): number { return x + 1; }

let x = 0;
for (let i = 0; i < 5_000_000; i = i + 1) {
    x = inc(x);
}
print x;
```

Metrics:

- Call overhead per invocation.
- Stack frame and return cost.

---

- **Array operations**

```kio
let xs = [0, 0, 0, 0, 0];
for (let i = 0; i < 2_000_000; i = i + 1) {
    xs[0] = xs[0] + 1;
}
print xs[0];
```

Metrics:

- Cost of `ARRAY_GET` / `ARRAY_SET`, bounds checks, and `ObjArray` access.

---

### 2. Vector and numeric benchmarks

Built around the `vector` module and `ObjArray` / `kio_vec_view`.

- **Vector add / mul / div**

```kio
let a = [/* N doubles */];
let b = [/* N doubles */];
let out = [0, 0, 0, 0 /* ... */];

// Warm up JIT / caches
for i in range(10) {
    vector.add(a, b, out);
}

// Timed region
for i in range(1000) {
    vector.add(a, b, out);
}
```

Sizes:

- N = 1K, 16K, 1M elements.

Metrics:

- Throughput in elements/second.
- Speedup from SIMD (AVX2) vs scalar path.

Implementation plan:

- Today: benchmarks call directly into `vectorized_*` functions (C++).
- With C core: update bindings to call `kio_vec_add/sub/mul/div` and compare.

---

- **Dot product and normalization**

```kio
let a = [/* N doubles */];
let b = [/* N doubles */];
let dot = vector.dot(a, b);
print dot;

let norm = vector.normalize(a);
print norm[0];
```

Metrics:

- Time per dot product / normalization.
- Numerical stability and correctness checks.

---

### 3. Native module boundary benchmarks

Measure overhead of crossing from KIO into C/C++ modules.

- **Crypto hashing**

```kio
let data = "some reasonably long input string ...";

for (let i = 0; i < 1_000_000; i = i + 1) {
    let h = crypto.sha256(data);
}
```

Focus:

- Call overhead (`CALL` opcode, name lookup, native invocation).
- Hash implementation cost (placeholder vs real SHA-256).

---

- **Database set/get**

```kio
for i in range(100_000) {
    db.set("key" + str(i), "value" + str(i));
}

for i in range(100_000) {
    let v = db.get("key" + str(i));
}
```

Focus:

- Native calls and string boxing/unboxing (`ObjString`).
- Hashed lookup in `KioDB`.

---

- **HTTP request handling (optional macrobenchmark)**

Drive `src/network/http_server.cpp` under load and measure:

- Requests/second for trivial responses.
- Overhead of parsing, array construction, and response formatting.

---

### 4. End-to-end macrobenchmarks

Small but realistic programs combining:

- Parsing.
- VM execution.
- Vector math.
- Native modules (crypto, db, net).

Examples:

- Numerical simulation loop with occasional I/O.
- Simple key/value cache server using `db` and `net` modules.

---

## Benchmark harness design

### Layout

- Benchmarks live under:
  - `tests/benchmarks/` for KIO source files and harness scripts.
  - `docs/benchmarks.md` (this document) for design and descriptions.

### Execution modes

1. **CLI-driven**:
   - Use the `kio` binary (`src/core/main.cpp`) to run `.kio` files:
     - `kio tests/benchmarks/vm_arith.kio`
   - Wrap calls with `/usr/bin/time` or similar tools.

2. **CTest integration**:
   - Add CTest entries that run benchmarks with environment flags:
     - `KIO_ENGINE=cpp` vs `KIO_ENGINE=c_core`.
   - Collect timing output in machine-readable format (e.g. JSON or CSV).

3. **Dedicated benchmark binary (future)**:
   - A small `kio_bench` driver that:
     - Loads a `.kio` file or embedded source string.
     - Selects engine (`cpp` vs `c_core`).
     - Runs the benchmark N times and reports aggregated statistics.

### Engine selection

To compare C++ VM and C core:

- Introduce a configuration option (CLI flag or env var), e.g.:
  - `kio --engine=cpp`
  - `kio --engine=c_core`
  - or `KIO_ENGINE=cpp|c_core` environment variable read by `Config::fromEnv()`.
- Wiring:
  - `engine=cpp`:
    - Current path: `Lexer` → `Parser` → `Compiler` → `kio::VM`.
  - `engine=c_core`:
    - `Lexer` → `Parser` → `Compiler` → `kio_chunk` (C core) → `kio_vm_execute`.

---

## Metrics and tooling

- **Primary metrics**
  - Wall-clock time (ms).
  - Iterations/sec or operations/sec.
  - For vector workloads: elements/sec and GB/sec.
- **Secondary metrics**
  - CPU cycles (via `perf stat`).
  - Cache misses and branch mispredictions (for VM loops).
  - Memory usage (RSS).

Recommended tools (Linux):

- `time` / `hyperfine` for wall-clock timing.
- `perf` for cycle and cache-level statistics.
- `valgrind` / `massif` for memory profile runs (less frequently).

---

## Correctness testing

Benchmarks should double as **tests**:

- Each benchmark should:
  - Print a final result or checksum.
  - Have an expected value asserted by the harness (either via CTest or a small comparison script).
- A single source file can serve both:
  - `tests/benchmarks/vm_arith.kio` (used for performance and for verifying VM behavior).

Where possible:

- Compare C++ VM vs C core results bit-for-bit.
- For floating-point workloads, use tolerances (`|a - b| < epsilon`) when necessary.

---

## Roadmap

1. **Phase 1 – KIO-only benchmarks**
   - Add KIO benchmark programs under `tests/benchmarks/`.
   - Run them using the existing C++ VM only.
2. **Phase 2 – C core hookup**
   - Implement `kio_vm` and `kio_chunk` wrappers around the existing C++ VM.
   - Add `--engine`/`KIO_ENGINE` switch.
   - Run all benchmarks with both engines and record baselines.
3. **Phase 3 – C core optimization**
   - Port the hot `VM::run` loop and vector operations to C.
   - Use benchmarks to guide low-level optimizations (SIMD, memory layout).
4. **Phase 4 – CI integration**
   - Add benchmarks to CI as **performance jobs** (not gating functional correctness).
   - Track trends over time to detect regressions.

