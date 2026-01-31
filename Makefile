# Copyright (c) 2026 Dipanjan Dhar
# SPDX-License-Identifier: GPL-3.0-only

# KIO Programming Language - Professional Build System
.PHONY: all clean install test package lsp docs benchmark

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
<<<<<<< HEAD
		-DKIO_ENABLE_PARALLEL=ON\
=======
		-DKIO_ENABLE_PARALLEL=ON \
>>>>>>> 202e6ff (Add install script, update Makefile, modify parallel_test.kio)
		-DKIO_ENABLE_LTO=ON \
		-DKIO_ENABLE_NATIVE_ARCH=ON \
		-DKIO_ENABLE_FAST_MATH=ON

kio: configure
	cd $(BUILD_DIR) && $(MAKE) -j$(PARALLEL_JOBS) kio

# Installation
install: kio
	cd $(BUILD_DIR) && sudo $(MAKE) install

uninstall:
	cd $(BUILD_DIR) && sudo $(MAKE) uninstall 2>/dev/null || true

# Testing
test: kio
	./$(BUILD_DIR)/kio math_test.kio
	cd $(BUILD_DIR) && ctest --output-on-failure

# Cleanup
clean:
	rm -rf $(BUILD_DIR)
	rm -rf build
