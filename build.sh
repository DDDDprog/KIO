#!/bin/bash
# Copyright (c) 2025 Dipanjan Dhar
# SPDX-License-Identifier: GPL-3.0-only


# Simple build script for KIO programming language
set -e

echo "Building KIO Programming Language..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring build..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17 \
    -DKIO_ENABLE_JIT=OFF \
    -DKIO_BUILD_LSP=ON

# Build the project
echo "Compiling..."
make -j$(nproc)

echo "Build completed successfully!"
echo "Executables:"
echo "  - kio: Main interpreter"
echo "  - kio-lsp: Language Server Protocol server"

# Run a simple test
if [ -f "../examples/hello.kio" ]; then
    echo ""
    echo "Running test..."
    ./kio ../examples/hello.kio
fi
