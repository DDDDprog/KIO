#!/usr/bin/env python3
# Copyright (c) 2026 Dipanjan Dhar
# KIO Super Build System

import os
import sys
import subprocess
import shutil

def build():
    print("🔥 Building KIO Ultra-Fast Engine...")
    
    if not os.path.exists("build_final"):
        os.makedirs("build")
    
    os.chdir("build")
    
    # Enable all performance flags
    cmake_cmd = [
        "cmake", "..",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DKIO_ENABLE_LTO=ON",
        "-DKIO_ENABLE_NATIVE_ARCH=ON",
        "-DKIO_ENABLE_FAST_MATH=ON",
        "-DKIO_ENABLE_JIT=OFF",
        "-DKIO_BUILD_LSP=OFF"
    ]
    
    ret = subprocess.call(cmake_cmd)
    if ret != 0:
        print("❌ CMake configuration failed.")
        return
    
    # Use all CPU cores for building
    jobs = str(os.cpu_count() or 4)
    ret = subprocess.call(["make", "-j" + jobs])
    if ret != 0:
        print("❌ Compilation failed.")
        return

    print("🚀 Success! KIO is ready.")
    print("Run './kio' for REPL or './kio test.kio' to execute a script.")

if __name__ == "__main__":
    build()
