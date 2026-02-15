#!/bin/bash
# Copyright (c) 2026 Dipanjan Dhar
# SPDX-License-Identifier: GPL-3.0-only

# Simple build script for Axeon programming language
set -e

echo "Building Axeon Programming Language..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring build..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17 \
    -DKIO_ENABLE_JIT=ON \
    -DKIO_BUILD_LSP=ON \
    -DKIO_ENABLE_PARALLEL=ON \
    -DKIO_ENABLE_LTO=ON \
    -DKIO_ENABLE_NATIVE_ARCH=ON \
    -DKIO_ENABLE_FAST_MATH=ON

# Build the project
echo "Compiling..."
make -j$(nproc)

echo "Build completed successfully!"
echo "Executables:"
echo "  - axeon: Main interpreter"
echo "  - axeon-lsp: Language Server Protocol server"

# Run a simple test
if [ -f "../examples/hello.axe" ]; then
    echo ""
    echo "Running test..."
    ./axeon ../examples/hello.axe
fi
