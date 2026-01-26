#!/bin/bash
# Install KIO on Debian/Ubuntu
sudo apt-get update
sudo apt-get install -y cmake g++ build-essential
cd ..
cmake -B build -DKIO_BUILD_LSP=ON
cmake --build build -j$(nproc)
sudo cmake --install build
echo "KIO installed successfully!"
