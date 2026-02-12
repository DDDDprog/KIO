# Copyright (c) 2026 Dipanjan Dhar
# SPDX-License-Identifier: GPL-3.0-only

# KIO Programming Language - Professional Build System
.PHONY: all clean install test test-all package lsp docs benchmark \
        linux-x64 windows-x64 mac-x64

# Build Configuration
BUILD_TYPE ?= Release
BUILD_DIR = build
INSTALL_PREFIX ?= /usr/local
PARALLEL_JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

all: kio

# Configure and build main project
configure:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. \
		-G "Unix Makefiles" \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
		-DKIO_BUILD_LSP=ON \
		-DKIO_ENABLE_JIT=ON \
		-DKIO_ENABLE_PARALLEL=ON \
		-DKIO_ENABLE_LTO=ON \
		-DKIO_ENABLE_NATIVE_ARCH=ON \
		-DKIO_ENABLE_FAST_MATH=ON

kio: configure
	cd $(BUILD_DIR) && $(MAKE) -j$(PARALLEL_JOBS) kio

# Cross-platform builds
linux-x64:
	@echo "Building for Linux x64..."
	@mkdir -p build-linux
	cd build-linux && cmake .. -DCMAKE_BUILD_TYPE=Release
	cd build-linux && $(MAKE) -j$(PARALLEL_JOBS)

windows-x64:
	@echo "Building for Windows x64 (MinGW)..."
	@mkdir -p build-windows
	cd build-windows && cmake .. -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_SYSTEM_NAME=Windows \
		-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
		-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
	cd build-windows && $(MAKE) -j$(PARALLEL_JOBS)

mac-x64:
	@echo "Building for macOS x64..."
	@mkdir -p build-mac
	@echo "Note: This requires a macOS environment or osxcross toolchain."
	cd build-mac && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Darwin
	cd build-mac && $(MAKE) -j$(PARALLEL_JOBS)

# Installation
install: kio
	cd $(BUILD_DIR) && sudo $(MAKE) install

uninstall:
	cd $(BUILD_DIR) && sudo $(MAKE) uninstall 2>/dev/null || true

# Testing
test: kio
	./$(BUILD_DIR)/kio examples/02_math.kio
	cd $(BUILD_DIR) && ctest --output-on-failure

test-all: kio
	@echo "Running all system tests..."
	./$(BUILD_DIR)/kio examples/01_basics.kio
	./$(BUILD_DIR)/kio examples/02_math.kio
	./$(BUILD_DIR)/kio examples/03_branching.kio
	./$(BUILD_DIR)/kio examples/04_loops.kio
	./$(BUILD_DIR)/kio examples/08_fizzbuzz.kio
	./$(BUILD_DIR)/kio examples/09_fibonacci.kio
	./$(BUILD_DIR)/kio examples/final_test.kio

# Cleanup
clean:
	rm -rf $(BUILD_DIR)
	rm -rf build-linux
	rm -rf build-windows
	rm -rf build-mac
	rm -rf build
