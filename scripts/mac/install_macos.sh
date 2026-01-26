#!/bin/bash
# Install KIO on Mac OS
if ! command -v brew &> /dev/null
then
    echo "Homebrew not found. Please install Homebrew first."
    exit
fi
brew install cmake gcc
cd ..
cmake -B build
cmake --build build -j$(sysctl -n hw.ncpu)
sudo cmake --install build
echo "KIO installed on Mac OS!"
