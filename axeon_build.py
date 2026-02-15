#!/usr/bin/env python3
# Copyright (c) 2026 Dipanjan Dhar
# Axeon Super Build System

import os
import sys
import subprocess
import shutil

def build():
    print("🔥 Building Axeon Ultra-Fast Engine...")
    
    if not os.path.exists("build"):
        os.makedirs("build", exist_ok=True)
    
    os.chdir("build")
    
    # Enable all performance flags
    cmake_cmd = [
        "cmake", "..",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DAXEON_ENABLE_LTO=ON",
        "-DAXEON_ENABLE_NATIVE_ARCH=ON",
        "-DAXEON_ENABLE_FAST_MATH=ON",
        "-DAXEON_ENABLE_JIT=ON",
        "-DAXEON_BUILD_LSP=OFF"
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

    print("🚀 Success! Axeon is ready.")
    print("Run './axeon' for REPL or './axeon test.axe' to execute a script.")

if __name__ == "__main__":
    build()
