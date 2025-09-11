# Copyright (c) 2025 Dipanjan Dhar
# SPDX-License-Identifier: GPL-3.0-only

# KIO Programming Language - Professional Build System
.PHONY: all clean install test package lsp vscode-ext docs benchmark

# Build Configuration
BUILD_TYPE ?= Release
BUILD_DIR = build
INSTALL_PREFIX ?= /usr/local
PARALLEL_JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Platform Detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM = linux
    PACKAGE_FORMATS = DEB RPM AppImage
endif
ifeq ($(UNAME_S),Darwin)
    PLATFORM = macos
    PACKAGE_FORMATS = DragNDrop
endif
ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    PACKAGE_FORMATS = ZIP NSIS WIX
endif

all: kio kio-lsp

# Configure and build main project
configure:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
		-DKIO_BUILD_LSP=ON \
		-DKIO_ENABLE_JIT=ON \
		-DKIO_ENABLE_PARALLEL=ON

kio: configure
	cd $(BUILD_DIR) && make -j$(PARALLEL_JOBS) kio

kio-lsp: configure
	cd $(BUILD_DIR) && make -j$(PARALLEL_JOBS) kio-lsp

# Development targets
debug:
	@$(MAKE) BUILD_TYPE=Debug all

release:
	@$(MAKE) BUILD_TYPE=Release all

# Testing
test: kio
	cd $(BUILD_DIR) && ctest --output-on-failure -j$(PARALLEL_JOBS)

benchmark: kio
	@echo "Running KIO performance benchmarks..."
	./$(BUILD_DIR)/kio examples/performance_test.kio
	@echo "Comparing with other languages..."
	python3 benchmarks/compare.py

# Installation
install: all
	cd $(BUILD_DIR) && make install

uninstall:
	cd $(BUILD_DIR) && make uninstall 2>/dev/null || true

# Packaging
package: all
	cd $(BUILD_DIR) && cpack -G "$(PACKAGE_FORMATS)"

# LSP and Editor Support
vscode-ext:
	@echo "Building VS Code extension..."
	cd lsp && npm install && npm run compile
	cd lsp && vsce package

# Documentation
docs:
	@echo "Generating documentation..."
	doxygen docs/Doxyfile 2>/dev/null || echo "Doxygen not found, skipping docs"

# Cleanup
clean:
	rm -rf $(BUILD_DIR)
	rm -rf lsp/out
	rm -rf lsp/node_modules

# Development helpers
format:
	find src include -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

lint:
	find src include -name "*.cpp" -o -name "*.hpp" | xargs clang-tidy

# Quick development cycle
dev: clean debug test

# Show build information
info:
	@echo "KIO Build Information:"
	@echo "  Platform: $(PLATFORM)"
	@echo "  Build Type: $(BUILD_TYPE)"
	@echo "  Build Directory: $(BUILD_DIR)"
	@echo "  Install Prefix: $(INSTALL_PREFIX)"
	@echo "  Parallel Jobs: $(PARALLEL_JOBS)"
	@echo "  Package Formats: $(PACKAGE_FORMATS)"
