# Axeon Programming Language

<p align="center">
  <img src="docs/image.jpg" width="100%"/>
</p>

Axeon is an ultra-fast, modern, lightweight programming language designed for high-performance computation, parallel workloads, and JIT-enabled execution. It combines the simplicity of Python/JavaScript with the speed of C++/Rust.

---

## Features

- **High-Performance VM**: Stack-based virtual machine with computed gotos for 30% faster execution
- **Just-In-Time Compilation (JIT)**: Optional high-speed execution for hot paths
- **Mark-and-Sweep GC**: Efficient garbage collection for long-running processes
- **Parallel Execution**: Native multi-threading with `parallel` blocks
- **Cross-platform**: Linux, macOS, Windows, iOS, Android, WASM supported
- **SIMD Vector Support**: Hardware-accelerated math operations (AVX2/NEON)
- **Native Modules**: Crypto, GPU, GUI, Database, Network, BigInt support
- **Standard Library**: Math, system queries, file I/O, modules
- **VS Code Integration**: Syntax highlighting, snippets, IntelliSense, LSP support
- **Fast Math & Optimizations**: Link-time optimization, fast-math, loop unrolling

---

## Quick Start

### 1️⃣ Build from Source

```bash
# Clone the repository
git clone https://github.com/DDDDprog/KIO.git
cd KIO
# Build with Makefile
make kio

# Or use the build script
./build.sh

# Or use Python build system
python3 axeon_build.py
```

### 2️⃣ Run Your First Program

```bash
# Run an example
./build/axeon examples/hello.axe

# Or use the built binary
./axeon examples/hello.axe
```

**Example program (hello.axe):**
```axeon
let x = 41 + 1;
print x;
print "Axeon";
```

---

## Installation

### Linux/macOS

```bash
# Build the project
make kio

# Install system-wide
sudo make install

# Or run from current directory
./build/axeon your_script.axe
```

### Windows

```bash
# Build with MinGW
bash build_windows.sh

# Run
./build_windows/axeon.exe your_script.axe
```

---

## Documentation

### Language Basics

**Variables:**
```axeon
let x = 42;
let name = "Axeon";
let pi = 3.14159;
```

**Constants:**
```axeon
const MAX_SIZE = 100;
const PI = 3.14159;
```

**Functions:**
```axeon
function fib(n: number): number {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

print fib(10); // 55
```

**Classes:**
```axeon
class Animal {
    function speak() {
        print "...";
    }
}

class Dog extends Animal {
    function speak() {
        print "Woof!";
    }
}
```

**Modules:**
```axeon
// math.axe
module math {
    export function square(x) {
        return x * x;
    }
}

// main.axe
import math from "math.axe";
print math.square(5); // 25
```

### Control Flow

```axeon
// If-Else
if (score >= 90) {
    print "A";
} else {
    print "B";
}

// While Loop
while (count > 0) {
    print count;
    count = count - 1;
}

// For Loop
for (let i = 0; i < 10; i = i + 1) {
    print i;
}
```

### Arrays and Strings

```axeon
let numbers = [1, 2, 3, 4, 5];
print numbers[0];

let greeting = "Hello";
let message = greeting + " World!";
```

### Exception Handling

```axeon
try {
    // Risky code
} catch (e) {
    print "Error: " + e;
} finally {
    print "Done";
}
```

### Parallel Execution

```axeon
parallel {
    // Runs in parallel
    let result = heavy_computation();
}
```

### System Queries

```axeon
print sys "os_name";
print sys "cpu_model";
print sys "time";
```

---

## Built-in Functions

### Output
```axeon
print "Hello";
println "With newline";
```

### Type Conversion
```axeon
str(value)      // Convert to string
int_func(value) // Convert to integer
float_func(value) // Convert to float
len(array)      // Get length
type(value)     // Get type
```

### Math
```axeon
abs_func(-5)    // Absolute value
min_func(a, b)  // Minimum
max_func(a, b)  // Maximum
pow_func(a, b)  // Power
sqrt_func(x)    // Square root
```

### String
```axeon
upper("hello") // "HELLO"
lower("HELLO") // "hello"
substr("Hello", 0, 3) // "Hel"
split("a,b,c", ",")   // ["a","b","c"]
join(["a","b"], "-")  // "a-b"
```

---

## Native Modules

### Crypto
```axeon
let hash = crypto.sha256("data");
```

### Database
```axeon
db.set("key", "value");
let val = db.get("key");
```

### Vector Math
```axeon
let a = [1.0, 2.0, 3.0];
let b = [4.0, 5.0, 6.0];
let dot = vector.dot(a, b);
```

### Network
```axeon
let server = net.server_init(8080);
```

### BigInt
```axeon
let big = bigint.add("123", "456");
```

---

## Build Options

### Makefile Targets

```bash
make kio           # Build main interpreter
make configure     # Configure CMake
make install       # Install system-wide
make test          # Run tests
make test-all      # Run all examples
make clean         # Clean build artifacts
make linux-x64    # Cross-compile for Linux
make windows-x64  # Cross-compile for Windows
make mac-x64     # Cross-compile for macOS
```

### CMake Options

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DKIO_ENABLE_JIT=ON \
  -DKIO_ENABLE_PARALLEL=ON \
  -DKIO_ENABLE_LTO=ON \
  -DKIO_ENABLE_NATIVE_ARCH=ON \
  -DKIO_BUILD_LSP=ON
```

---

## Examples

| Example | Description |
|---------|-------------|
| `hello.axe` | Hello World |
| `01_basics.axe` | Variables and printing |
| `02_math.axe` | Arithmetic operations |
| `03_branching.axe` | If-else statements |
| `04_loops.axe` | While loops |
| `05_scopes.axe` | Block scopes |
| `06_system_info.axe` | System queries |
| `07_strings_concat.axe` | String operations |
| `08_fizzbuzz.axe` | FizzBuzz challenge |
| `09_fibonacci.axe` | Fibonacci sequence |
| `10_io_save_load.axe` | Save/load state |
| `11_import_demo.axe` | Module imports |
| `parallel_test.axe` | Parallel execution |
| `benchmark_fast.axe` | Performance benchmark |

---

## VS Code Extension

Install the Axeon VS Code extension for:
- Syntax highlighting for `.axe` files
- IntelliSense and auto-completion
- Code snippets
- Go to definition
- Hover information
- Error diagnostics

### Installation

```bash
cd vscode-extension
npm install
npm run compile
code --install-extension axeon-language-*.vsix
```

---

## Performance

Axeon is designed for speed:

| Feature | Benefit |
|---------|---------|
| Computed Gotos | 30% faster opcode dispatch |
| JIT Compilation | Hot path optimization |
| SIMD Vector Ops | Hardware-accelerated math |
| Adaptive GC | Low-latency memory management |
| Parallel Blocks | Multi-threaded execution |

---

## Contributing

Contributions are welcome! Please read our [Contributing Guidelines](CONTRIBUTING.md).

---

## License

Copyright (c) 2026 Dipanjan Dhar / Google DeepMind Agentic Coding Team

GPL-3.0 License - See [LICENSE](LICENSE) file for details.

---

<p align="center">
  <strong>Axeon</strong> - Fast, Modern, Simple
</p>
