# KIO Programming Language

KIO is an ultra-fast, modern, lightweight programming language designed for high-performance computation, parallel workloads, and JIT-enabled execution. It combines the simplicity of Python/JavaScript with the speed of C++/Rust.

![KIO Logo](docs/image.jpg)

---

## Features

- **Lightweight Interpreter:** Minimal overhead, easy to build and run.
- **Just-In-Time Compilation (JIT):** Optional high-speed execution.
- **Parallel Execution:** Multi-threaded computation for modern CPUs.
- **Cross-platform:** Linux, macOS, Windows supported.
- **Standard Library:** Math, system queries, file I/O, modules.
- **VS Code Integration:** Syntax highlighting, snippets, tasks.
- **Fast Math & Optimizations:** Link-time optimization, fast-math, loop unrolling, native CPU optimizations.

---

## Quick Start

### 1️⃣ Build from Source

```bash
# Clean previous builds
bash scripts/clean_build.sh

# Build KIO with optional features
cmake -S . -B build \
  -DKIO_ENABLE_JIT=ON \
  -DKIO_ENABLE_PARALLEL=ON \
  -DKIO_BUILD_LSP=ON

# Compile
cmake --build build -j
```
``` bash
2️⃣ Run Example
./build/kio examples/hello.kio
Example hello.kio:

let x = 41 + 1;
print x;
print "kio";
```
```
3️⃣ Install System-wide
sudo cmake --install build


