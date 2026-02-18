# Axeon Programming Language

<p align="center">
  <img src="docs/image.jpg" alt="Axeon Logo" width="150"/>
</p>

Axeon (KIO) is an ultra-fast, modern, lightweight programming language designed for high-performance computation, parallel workloads, and JIT-enabled execution. It combines the expressive syntax of high-level languages with the raw performance of a low-level runtime.

---

## 🚀 Key Features

- **High-Performance VM**: Stack-based virtual machine with computed gotos for rapid opcode dispatch.
- **Just-In-Time Compilation (JIT)**: Integrated LLVM-based JIT for hot path optimization.
- **Expanded Keyword Set**: Over 50 keywords including `fn`, `mut`, `match`, `struct`, `interface`, and more.
- **Rich Type System**: Support for fixed-size integers (`i8`..`i128`, `u8`..`u128`), floating point, and native types.
- **Massive Standard API**: 100+ built-in functions covering I/O, Math, Crypto (SHA256, AES), JSON, Network, and System.
- **Parallel Execution**: First-class support for parallel workloads.
- **Modern Tooling**: Python-based build automation and VS Code integration.

---

## 🛠️ Quick Start

### 1. Build from Source
The project uses a unified Python-based build system that handles CMake configuration and compilation automatically.

```bash
# Clone the repository
git clone https://github.com/DDDDprog/KIO.git
cd KIO

# Build the project (Release mode by default)
python3 build/axeon_build.py

# Or perform a clean build
python3 build/axeon_build.py --clean
```

### 2. Run Your First Program
Build artifacts are placed in the `dist/` directory.

```bash
# Run the REPL
./dist/axeon

# Run a script
./dist/axeon examples/hello.axe
```

**Example (hello.axe):**
```axeon
fn main() {
    let message = "Hello, Axeon!";
    println message;
    
    mut sum = 0;
    for (let i = 0; i < 10; i++) {
        sum += i;
    }
    println "Sum: " + sum;
}
main();
```

---

## 📖 Documentation

### Language Syntax
Axeon now supports a comprehensive set of modern keywords:
- **Control Flow**: `if`, `else`, `elif`, `then`, `end`, `switch`, `case`, `break`, `continue`, `match`.
- **Functions & Modules**: `fn`, `function`, `return`, `import`, `export`, `module`, `use`.
- **Data & Types**: `struct`, `class`, `enum`, `trait`, `impl`, `type`, `mut`, `const`.
- **System & Memory**: `unsafe`, `extern`, `sizeof`, `typeof`, `panic`, `assert`.

### Built-in API
The runtime provides an extensive standard library registered as native functions:
- **Crypto**: `crypto.sha256()`, `crypto.aes_encrypt()`, `crypto.aes_decrypt()`.
- **Math**: `abs()`, `sqrt()`, `pow()`, `min()`, `max()`, `sin()`, `cos()`, etc.
- **System**: `sys "time"`, `sys "os_name"`, `sys "arch"`, `sys "kio_version"`.
- **Data Structures**: Full support for Vectorized operations and JSON parsing stubs.

---
## 🧩 VS Code Integration
An official extension is available in the `vscode-extension/` directory, providing syntax highlighting, snippets, and LSP support.

---

## 📜 Contributing & License
Copyright (c) 2026 Dipanjan Dhar / Google DeepMind Agentic Coding Team.
GPL-3.0 License - See [LICENSE](LICENSE) file for details.

---

<p align="center">
  <strong>Axeon</strong> - Fast, Modern, Simple
</p>
