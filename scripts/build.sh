#!/bin/bash
# Copyright (c) 2025 Dipanjan Dhar
# SPDX-License-Identifier: Zeo-3.0-only


set -e

echo "Building Axeon Programming Language..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DAXEON_BUILD_LSP=OFF \
    -DAXEON_ENABLE_JIT=OFF \
    -DAXEON_ENABLE_PARALLEL=OFF \
    -DAXEON_ENABLE_LTO=OFF

# Build with all available cores
make -j$(nproc)

echo "Build completed successfully!"
echo "Run 'sudo make install' to install Axeon system-wide"
echo "Or run './axeon ../examples/hello.axe' to test"
