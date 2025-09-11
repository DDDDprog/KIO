#!/bin/bash

set -e

echo "Installing KIO Programming Language dependencies for Ubuntu..."

# Update package list
sudo apt update

# Install build dependencies
sudo apt install -y \
    build-essential \
    cmake \
    pkg-config \
    libllvm-14-dev \
    llvm-14-dev \
    clang-14 \
    libnlohmann-json3-dev \
    git

# Set environment variables for LLVM
export LLVM_CONFIG=/usr/bin/llvm-config-14
export CC=clang-14
export CXX=clang++-14

echo "Dependencies installed successfully!"
echo "Now run './scripts/build.sh' to build KIO"
