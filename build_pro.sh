#!/bin/bash
# KIO Professional Build Script
# Optimized for maximum performance

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_pro"

echo "========================================"
echo "  KIO Professional Build"
echo "========================================"
echo ""

# Clean previous build
echo "[1/5] Cleaning previous build..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure with aggressive optimizations
echo "[2/5] Configuring with LLVM 21 and optimizations..."
cd "$BUILD_DIR"

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native -ffast-math -funroll-loops -fomit-frame-pointer" \
    -DKIO_ENABLE_JIT=ON \
    -DKIO_ENABLE_LTO=ON \
    -DKIO_ENABLE_FAST_MATH=ON \
    -DKIO_ENABLE_NATIVE_ARCH=ON \
    -DKIO_ENABLE_PARALLEL=ON \
    "$SCRIPT_DIR"

# Build with all cores
echo "[3/5] Building with $(nproc) parallel jobs..."
make -j$(nproc)

# Verify build
echo "[4/5] Verifying build..."
if [ -f "$BUILD_DIR/kio" ]; then
    echo "✅ KIO executable: $BUILD_DIR/kio"
    "$BUILD_DIR/kio" --version
else
    echo "❌ Build failed!"
    exit 1
fi

# Run quick test
echo "[5/5] Running quick test..."
"$BUILD_DIR/kio" "$SCRIPT_DIR/examples/hello.kio" > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✅ Tests passed"
else
    echo "⚠️  Tests failed"
fi

echo ""
echo "========================================"
echo "  Build Complete!"
echo "========================================"
echo "KIO binary: $BUILD_DIR/kio"
echo ""
echo "Run benchmarks:"
echo "  $BUILD_DIR/kio $SCRIPT_DIR/examples/benchmark.kio"
echo ""
