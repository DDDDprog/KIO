# KIO Build Instructions

## Quick Build (Ubuntu/Linux)

### Prerequisites
\`\`\`bash
sudo apt update
sudo apt install build-essential cmake pkg-config
\`\`\`

### Build Steps
\`\`\`bash
# Make build script executable
chmod +x build.sh

# Run the build
./build.sh
\`\`\`

### Alternative Manual Build
\`\`\`bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17
make -j$(nproc)
\`\`\`

## Build Options

- `KIO_ENABLE_JIT=ON/OFF` - Enable LLVM JIT compilation (requires LLVM)
- `KIO_BUILD_LSP=ON/OFF` - Build Language Server Protocol server
- `KIO_BUILD_STATIC=ON/OFF` - Build static executable
- `CMAKE_BUILD_TYPE=Release/Debug` - Build configuration

## Troubleshooting

If you encounter compilation errors:

1. **C++ Standard Issues**: Make sure you're using C++17
2. **Missing Dependencies**: Install required packages
3. **LLVM Issues**: Disable JIT with `-DKIO_ENABLE_JIT=OFF`

## Testing

After building, test with:
\`\`\`bash
cd build
./kio ../examples/hello.kio
