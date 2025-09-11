#!/bin/bash

set -e

echo "Building KIO Programming Language..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DKIO_BUILD_LSP=OFF \
    -DKIO_ENABLE_JIT=OFF \
    -DKIO_ENABLE_PARALLEL=OFF \
    -DKIO_ENABLE_LTO=OFF

# Build with all available cores
make -j$(nproc)

echo "Build completed successfully!"
echo "Run 'sudo make install' to install KIO system-wide"
echo "Or run './kio ../examples/hello.kio' to test"
