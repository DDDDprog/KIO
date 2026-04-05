╔══════════════════════════════════════════════════════════════════════════╗
║              AXEON - BUILD GUIDE WITH MODE SELECTION                     ║
╚══════════════════════════════════════════════════════════════════════════╝

## HOW TO BUILD AXEON WITH MODE SELECTION

### Step 1: Clone & Navigate to Project
```bash
git clone https://github.com/DDDDprog/KIO.git
cd KIO
```

### Step 2: Build from Source
```bash
# Automatic build (recommended)
bash scripts/build.sh

# Or manual CMake build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DAXEON_ENABLE_JIT=ON -DAXEON_ENABLE_PARALLEL=ON
make -j$(nproc)
cd ..
```

### Step 3: Install System-Wide (Optional)
```bash
cd build
sudo make install
cd ..

# Now you can run from anywhere
axeon script.axe
```

═══════════════════════════════════════════════════════════════════════════

## RUNNING AXEON IN DIFFERENT MODES

### ✅ Method 1: Command-Line Flags (Shorthand)

```bash
# DEFAULT: VM Mode (stack-based bytecode)
axeon script.axe
./build/axeon script.axe

# Interpreter Mode (tree-walking AST)
axeon --interp script.axe
./build/axeon --interp script.axe

# JIT Mode (LLVM compiled)
axeon --jit script.axe
./build/axeon --jit script.axe
```

### ✅ Method 2: Long-Form Flags

```bash
# VM mode
axeon --engine=vm script.axe

# Interpreter mode
axeon --engine=interpreter script.axe

# JIT mode
axeon --engine=jit script.axe
```

### ✅ Method 3: Environment Variables

```bash
# Set VM mode globally
export AXEON_ENGINE=vm
axeon script.axe

# Set Interpreter mode
export AXEON_ENGINE=interpreter
axeon script.axe

# Set JIT mode
export AXEON_ENGINE=jit
axeon script.axe

# Or inline with command
AXEON_ENGINE=interp axeon script.axe
AXEON_ENGINE=jit axeon script.axe
```

═══════════════════════════════════════════════════════════════════════════

## BUILD CONFIGURATION OPTIONS

### CMake Flags for Build Customization

```bash
cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DAXEON_ENABLE_JIT=ON          # Enable LLVM JIT (recommended)
  -DAXEON_ENABLE_PARALLEL=ON     # Enable parallel execution
  -DAXEON_ENABLE_LTO=ON          # Link-Time Optimization
  -DAXEON_BUILD_LSP=OFF          # Language Server Protocol support
  -DAXEON_NATIVE_ARCH=ON         # Optimize for native CPU
  -DAXEON_FAST_MATH=ON           # Fast math operations

make -j$(nproc)
```

### Quick Build Presets

```bash
# FASTEST BUILD (disable JIT)
bash scripts/build.sh
# Then edit scripts/build.sh and set -DAXEON_ENABLE_JIT=OFF

# MAXIMUM PERFORMANCE (all optimizations)
# Edit scripts/build.sh and set:
# -DAXEON_ENABLE_JIT=ON
# -DAXEON_ENABLE_LTO=ON
# -DAXEON_ENABLE_PARALLEL=ON
# -DAXEON_NATIVE_ARCH=ON
bash scripts/build.sh

# MINIMUM SIZE (production container)
# Set all ENABLE_* to OFF except JIT
bash scripts/build.sh
```

═══════════════════════════════════════════════════════════════════════════

## EXECUTION MODES EXPLAINED

┌─────────────────────────────────────────────────────────────────────────┐
│ MODE 1: VM (STACK-BASED BYTECODE) - DEFAULT                            │
├─────────────────────────────────────────────────────────────────────────┤
│ Run: axeon --vm script.axe                                              │
│ Or:  axeon script.axe  (no flag = VM by default)                        │
│                                                                          │
│ How it works:                                                           │
│  1. Lexer tokenizes source code                                         │
│  2. Parser builds Abstract Syntax Tree (AST)                            │
│  3. Compiler generates optimized bytecode                               │
│  4. Stack-based VM executes bytecode                                    │
│  5. JIT automatically compiles hot loops to native code                 │
│                                                                          │
│ Advantages:                                                             │
│  ✅ Fast startup time                                                    │
│  ✅ Compact memory footprint                                             │
│  ✅ Good for one-off scripts                                             │
│  ✅ Automatic JIT for performance-critical code                          │
│                                                                          │
│ Performance: 2nd fastest (after JIT on hot code)                        │
│ Use When: General-purpose scripts, quick startup                        │
└─────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────┐
│ MODE 2: INTERPRETER (TREE-WALKING) - DEBUG MODE                        │
├─────────────────────────────────────────────────────────────────────────┤
│ Run: axeon --interp script.axe                                          │
│ Or:  AXEON_ENGINE=interpreter axeon script.axe                          │
│                                                                          │
│ How it works:                                                           │
│  1. Lexer tokenizes source code                                         │
│  2. Parser builds Abstract Syntax Tree (AST)                            │
│  3. Direct AST traversal and evaluation (no bytecode)                   │
│  4. Interprets each node dynamically                                    │
│                                                                          │
│ Advantages:                                                             │
│  ✅ Easy debugging (can inspect AST nodes)                               │
│  ✅ Simple execution model                                               │
│  ✅ Good for development/learning                                        │
│  ✅ Flexible dynamic execution                                           │
│                                                                          │
│ Disadvantages:                                                          │
│  ❌ Slowest performance                                                  │
│  ❌ Higher memory overhead                                               │
│                                                                          │
│ Performance: Slowest (AST interpretation overhead)                      │
│ Use When: Debugging, development, educational purposes                 │
└─────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────┐
│ MODE 3: JIT (LLVM JUST-IN-TIME) - PERFORMANCE MODE                     │
├─────────────────────────────────────────────────────────────────────────┤
│ Run: axeon --jit script.axe                                             │
│ Or:  AXEON_ENGINE=jit axeon script.axe                                  │
│                                                                          │
│ How it works:                                                           │
│  1. Code starts in VM mode (bytecode execution)                         │
│  2. Profiler monitors hot paths                                         │
│  3. LLVM compiler invoked for frequently-executed code                  │
│  4. Native machine code replaces bytecode for hot sections               │
│  5. Continue execution at native speed                                  │
│                                                                          │
│ Advantages:                                                             │
│  ✅ Fastest performance (native code)                                    │
│  ✅ Automatic optimization                                               │
│  ✅ Near C-performance for hot code                                      │
│  ✅ Best for production workloads                                        │
│                                                                          │
│ Disadvantages:                                                          │
│  ❌ Longer startup (JIT compilation overhead)                            │
│  ❌ Memory overhead for compiled code                                    │
│  ❌ Potential for JIT bugs on complex code                               │
│                                                                          │
│ Performance: Fastest (for repeated execution)                           │
│ Use When: Production, benchmarks, long-running processes                │
└─────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════

## SELECTION MODE REFERENCE TABLE

┌───────────┬──────────────────────────┬────────────────┬──────────────────┐
│   Mode    │      Command             │    Performance │   Best For       │
├───────────┼──────────────────────────┼────────────────┼──────────────────┤
│ VM        │ axeon --vm script.axe    │   Good (2nd)   │ Scripts, default │
│ Interp    │ axeon --interp script.axe│  Slow (3rd)    │ Debug, learning  │
│ JIT       │ axeon --jit script.axe   │   Fast (1st)   │ Production, perf  │
└───────────┴──────────────────────────┴────────────────┴──────────────────┘

═══════════════════════════════════════════════════════════════════════════

## COMPLETE EXAMPLES

### Example 1: Build and Run in VM Mode (Default)
```bash
cd KIO
bash scripts/build.sh
./build/axeon examples/hello.axe
# Output: 42, axeon
```

### Example 2: Build and Run in Interpreter Mode
```bash
bash scripts/build.sh
./build/axeon --interp examples/hello.axe
# Output: 42, axeon
```

### Example 3: Install and Run with Different Modes
```bash
bash scripts/build.sh
cd build
sudo make install
cd ..

# Run in VM mode
axeon examples/fibonacci.axe

# Run in Interpreter mode
axeon --interp examples/fibonacci.axe

# Run in JIT mode
axeon --jit examples/fibonacci.axe

# Check configuration
axeon --info
```

### Example 4: Environment Variable Method
```bash
# Set VM mode
export AXEON_ENGINE=vm
axeon script.axe

# Set Interpreter mode
export AXEON_ENGINE=interpreter
axeon script.axe

# Inline (one-time)
AXEON_ENGINE=jit axeon script.axe
```

### Example 5: Help and Info Commands
```bash
axeon --help      # Display all options
axeon --info      # Show build configuration
axeon --version   # Show version info
axeon --benchmark # Run performance test
```

═══════════════════════════════════════════════════════════════════════════

## CHOOSING THE RIGHT MODE

🎯 USE VM MODE IF:
  • Running one-off scripts
  • Need fast startup time
  • Running on resource-constrained hardware
  • Building tools that use Axeon as embedded runtime
  • Default choice for most use cases

🐛 USE INTERPRETER MODE IF:
  • Debugging Axeon programs
  • Learning how the language works
  • Need full AST introspection
  • Teaching language internals
  • Building IDE/debugger tools

⚡ USE JIT MODE IF:
  • Running computationally intensive code
  • Performance is critical
  • Code runs in long loops/repeatedly
  • Building production applications
  • Benchmarking/comparing performance

═══════════════════════════════════════════════════════════════════════════

## ADVANCED: BUILDING FOR DIFFERENT ENVIRONMENTS

### For Docker Container (Minimal)
```dockerfile
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y build-essential cmake llvm-21 clang

WORKDIR /app
COPY . .

RUN bash scripts/build.sh && \
    cd build && \
    sudo make install && \
    cd .. && \
    rm -rf build  # Remove build artifacts to save space

ENTRYPOINT ["axeon"]
```

### For Windows (via Visual Studio)
```bash
./build_windows_vs.sh
# Generates VS solution, then:
# axeon_vs.exe script.axe           # VM mode
# set AXEON_ENGINE=interpreter && axeon_vs.exe script.axe
```

### For macOS
```bash
bash scripts/build.sh
# Works same as Linux, installs to /usr/local/bin/axeon
```

═══════════════════════════════════════════════════════════════════════════
