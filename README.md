<img src="KIO-main/docs/images/logo.jpg" alt="KIO Logo" width="16000" />

# KIO Programming Language

A lightweight, fast, modern language with a minimal interpreter and growing tooling.

- CLI interpreter: builds with CMake and installs to `/usr/local/bin/kio`.
- VS Code extension: separate `vscode-extension/` folder with syntax highlighting and snippets.
- Clean baseline build profile keeps the project always runnable; advanced features (LSP, JIT) can be enabled after alignment.

## Quick Start

- Build
  ```bash
  bash scripts/clean_build.sh
  bash scripts/build.sh
  ```
- Run
  ```bash
  ./build/kio examples/hello.kio
  ```
- Install (system-wide, no reconfigure)
  ```bash
  sudo cmake --install build
  ```

## Language Status

- Implemented
  - Variables: `let x = 42;`
  - Print: `print "hello";`
  - Arithmetic: `+ - * / %`
  - Comparisons: `== != < <= > >=` (return `1` or `0`)
  - Grouping: `( ... )`
  - Assignment: `x = x + 1;`
  - I/O: `save "state.json";` / `load "state.json";`
  - Import: `import "examples/hello.kio";`
  - System query: `sys "time";` (ms since epoch), `sys "cpu_model"`, `sys "mem_total_kb"`, `sys "disk_root_kb"`, `sys "os_name"`

- In progress / Roadmap
  - Blocks `{ ... }`, `if/else`, and `while` statements
  - Standard library modules
  - LSP server (hover, completion, diagnostics) with VS Code integration
  - JIT, parallel execution, semantic tokens

## Examples

- `examples/hello.kio`
  ```kio
  let x = 41 + 1;
  print x;
  print "kio";
  ```

- Simple comparisons
  ```kio
  print 3 < 4;
  print 5 == 5;
  print 6 != 7;
  ```

## VS Code Extension

The extension lives in `vscode-extension/` and uses a TextMate grammar for highlighting. Snippets include `kio-print`, `kio-let`, `kio-if`, `kio-while`.

- Development run
  1) Open `vscode-extension/` in VS Code
  2) Press `F5` to start the Extension Development Host
  3) Open a `.kio` file

- Packaging
  ```bash
  npm install -g vsce
  cd vscode-extension
  vsce package
  ```

- Logo
  - Source image: `docs/image.jpg`
  - For the extension marketplace icon, copy the image inside the extension package:
    ```bash
    mkdir -p vscode-extension/images
    cp docs/image.jpg vscode-extension/images/logo.jpg
    ```
    The extension `package.json` references `images/logo.jpg`.

## Build Profiles

- Stable (default via `scripts/build.sh`)
  - `KIO_BUILD_LSP=OFF`, `KIO_ENABLE_JIT=OFF`, `KIO_ENABLE_PARALLEL=OFF`, `KIO_ENABLE_LTO=OFF`
  - Goal: Always-works baseline

- Advanced
  - Enable LSP/JIT/Parallel once headers and sources are aligned. Use CMake options:
    ```bash
    cmake -S . -B build \
      -DKIO_BUILD_LSP=ON \
      -DKIO_ENABLE_JIT=ON \
      -DKIO_ENABLE_PARALLEL=ON
    cmake --build build -j
    ```

## Contributing

- See `CONTRIBUTING.md`
- Code style: keep the code warning-free and the baseline build green. Add tests/examples where possible.

## License

- GPL-3.0-only. See `LICENSE`.
