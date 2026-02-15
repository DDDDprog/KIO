# Copyright (c) 2026 Dipanjan Dhar
# SPDX-License-Identifier: GPL-3.0-only

# Axeon Programming Language - Professional Build System
.PHONY: all clean install test test-all docs benchmark rebuild help \
        linux-x64 windows-x64 mac-x64 release debug profile

# Build Configuration
BUILD_TYPE ?= Release
BUILD_DIR ?= build
INSTALL_PREFIX ?= /usr/local
PARALLEL_JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Executable names
AXEON ?= axeon
AXEON_LSP ?= axeon-lsp

# Default target
all: axeon

# Configure CMake
configure:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. \
		-G "Unix Makefiles" \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
		-DKIO_BUILD_LSP=ON \
		-DKIO_ENABLE_JIT=ON \
		-DKIO_ENABLE_PARALLEL=ON \
		-DKIO_ENABLE_LTO=ON \
		-DKIO_ENABLE_NATIVE_ARCH=ON \
		-DKIO_ENABLE_FAST_MATH=ON \
		-DKIO_ENABLE_WERROR=OFF

# Build Axeon interpreter
axeon: configure
	@cd $(BUILD_DIR) && $(MAKE) -j$(PARALLEL_JOBS) $(AXEON)

# Build Axeon with LSP server
axeon-full: configure
	@cd $(BUILD_DIR) && $(MAKE) -j$(PARALLEL_JOBS)
	@echo "Build complete!"
	@echo "  - $(AXEON): Main interpreter"
	@echo "  - $(AXEON_LSP): Language Server"

# Quick rebuild
rebuild:
	@$(MAKE) clean
	@$(MAKE) axeon

# Release build (optimized)
release:
	@$(MAKE) BUILD_TYPE=Release axeon

# Debug build (with symbols)
debug:
	@$(MAKE) BUILD_TYPE=Debug axeon

# Profile build (with profiling flags)
profile:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DKIO_ENABLE_JIT=ON \
		-DKIO_ENABLE_PARALLEL=ON
	@cd $(BUILD_DIR) && $(MAKE) -j$(PARALLEL_JOBS)

# Cross-platform builds
linux-x64:
	@echo "Building for Linux x64..."
	@mkdir -p build-linux
	@cd build-linux && cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_SYSTEM_NAME=Linux \
		-DKIO_ENABLE_NATIVE_ARCH=ON
	@cd build-linux && $(MAKE) -j$(PARALLEL_JOBS)
	@echo "Output: build-linux/$(AXEON)"

windows-x64:
	@echo "Building for Windows x64 (MinGW)..."
	@mkdir -p build-windows
	@cd build-windows && cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_SYSTEM_NAME=Windows \
		-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
		-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
		-DKIO_BUILD_STATIC=ON
	@cd build-windows && $(MAKE) -j$(PARALLEL_JOBS)
	@echo "Output: build-windows/$(AXEON).exe"

mac-x64:
	@echo "Building for macOS x64..."
	@mkdir -p build-mac
	@cd build-mac && cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_SYSTEM_NAME=Darwin \
		-DCMAKE_OSX_ARCHITECTURES=x86_64
	@cd build-mac && $(MAKE) -j$(PARALLEL_JOBS)
	@echo "Output: build-mac/$(AXEON)"

mac-arm64:
	@echo "Building for macOS ARM64 (Apple Silicon)..."
	@mkdir -p build-mac-arm64
	@cd build-mac-arm64 && cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_SYSTEM_NAME=Darwin \
		-DCMAKE_OSX_ARCHITECTURES=arm64
	@cd build-mac-arm64 && $(MAKE) -j$(PARALLEL_JOBS)
	@echo "Output: build-mac-arm64/$(AXEON)"

# Installation
install: axeon
	@cd $(BUILD_DIR) && $(MAKE) install

uninstall:
	@cd $(BUILD_DIR) && $(MAKE) uninstall 2>/dev/null || true

# Testing
test: axeon
	@echo "Running tests..."
	@./$(BUILD_DIR)/$(AXEON) examples/hello.axe || echo "Test failed"
	@cd $(BUILD_DIR) && ctest --output-on-failure || true

test-all: axeon
	@echo "Running all examples..."
	@./$(BUILD_DIR)/$(AXEON) examples/hello.axe
	@./$(BUILD_DIR)/$(AXEON) examples/01_basics.axe
	@./$(BUILD_DIR)/$(AXEON) examples/02_math.axe
	@./$(BUILD_DIR)/$(AXEON) examples/03_branching.axe
	@./$(BUILD_DIR)/$(AXEON) examples/04_loops.axe
	@./$(BUILD_DIR)/$(AXEON) examples/05_scopes.axe
	@./$(BUILD_DIR)/$(AXEON) examples/06_system_info.axe
	@./$(BUILD_DIR)/$(AXEON) examples/07_strings_concat.axe
	@./$(BUILD_DIR)/$(AXEON) examples/08_fizzbuzz.axe
	@./$(BUILD_DIR)/$(AXEON) examples/09_fibonacci.axe
	@echo "All examples passed!"

benchmark: axeon
	@echo "Running benchmark..."
	@./$(BUILD_DIR)/$(AXEON) examples/benchmark_fast.axe

# Documentation
docs:
	@echo "Generating documentation..."
	@mkdir -p docs/html
	@# Add documentation generation commands here

# Development helpers
dev: debug
	@echo "Development build complete. Run: ./$(BUILD_DIR)/$(AXEON)"

# Cleanup
clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf build-linux
	@rm -rf build-windows
	@rm -rf build-mac
	@rm -rf build-mac-arm64
	@rm -rf build-pro
	@echo "Clean complete!"

# Show help
help:
	@echo "Axeon Build System"
	@echo "=================="
	@echo ""
	@echo "Main targets:"
	@echo "  make axeon         - Build Axeon interpreter"
	@echo "  make axeon-full   - Build with LSP server"
	@echo "  make rebuild      - Clean and rebuild"
	@echo ""
	@echo "Build types:"
	@echo "  make release      - Optimized release build"
	@echo "  make debug        - Debug build with symbols"
	@echo "  make profile      - Build with profiling"
	@echo ""
	@echo "Cross-compilation:"
	@echo "  make linux-x64    - Build for Linux x64"
	@echo "  make windows-x64  - Build for Windows x64"
	@echo "  make mac-x64      - Build for macOS x64"
	@echo "  make mac-arm64    - Build for Apple Silicon"
	@echo ""
	@echo "Other targets:"
	@echo "  make install      - Install system-wide"
	@echo "  make test         - Run tests"
	@echo "  make test-all     - Run all examples"
	@echo "  make benchmark    - Run benchmark"
	@echo "  make docs         - Generate documentation"
	@echo "  make clean        - Clean build artifacts"
	@echo ""
	@echo "Environment variables:"
	@echo "  BUILD_TYPE        - Release/Debug (default: Release)"
	@echo "  BUILD_DIR         - Build directory (default: build)"
	@echo "  PARALLEL_JOBS     - Number of parallel jobs"
	@echo ""
	@echo "Examples:"
	@echo "  make release BUILD_DIR=mybuild"
	@echo "  make debug PARALLEL_JOBS=2"
