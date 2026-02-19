#!/bin/bash
# Build Axeon for Windows using Visual Studio
# This script creates a Visual Studio solution and builds the .exe file

set -e

echo "Building Axeon for Windows using Visual Studio..."

# Create build directory
mkdir -p build-vs
cd build-vs

# Generate Visual Studio solution
echo "Generating Visual Studio solution..."
cmake .. -G "Visual Studio 17 2026" -A x64 \
    -DCMAKE_BUILD_TYPE=Release \
    -DAXEON_BUILD_STATIC=ON \
    -DAXEON_ENABLE_JIT=OFF \
    -DAXEON_ENABLE_PARALLEL=OFF

# Build with Visual Studio
echo "Building with Visual Studio..."
cmake --build . --config Release --parallel $(nproc)

# Copy the executable to the root directory
echo "Copying executable..."
cp Release/axeon.exe ../axeon_vs.exe

# Strip symbols (if strip is available)
if command -v strip && [ -f ../axeon_vs.exe ]; then
    echo "Stripping symbols..."
    strip ../axeon_vs.exe
fi

echo "Build completed successfully!"
echo "Executable: axeon_vs.exe"
echo "You can now run: ./axeon_vs.exe ../examples/hello.axe"
