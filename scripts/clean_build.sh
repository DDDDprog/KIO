#!/bin/bash

# Clean build script that removes all build artifacts
set -e

echo "Cleaning KIO build artifacts..."

# Remove build directory
if [ -d "build" ]; then
    rm -rf build
    echo "Removed build directory"
fi

# Remove any CMake cache files
find . -name "CMakeCache.txt" -delete 2>/dev/null || true
find . -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null || true

echo "Clean completed!"
