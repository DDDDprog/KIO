#!/bin/bash
set -e

KIO_BIN="./build/kio"

if [ ! -x "$KIO_BIN" ]; then
    echo "KIO binary not found at $KIO_BIN. Build the project first (e.g. via build.sh)."
    exit 1
fi

if ! command -v javac >/dev/null 2>&1; then
    echo "javac not found on PATH. Java benchmarks cannot be run."
    exit 1
fi

if ! command -v java >/dev/null 2>&1; then
    echo "java runtime not found on PATH. Java benchmarks cannot be run."
    exit 1
fi

CLASS_DIR="build/java_bench"
SRC="examples/SimpleBenchmarkJava.java"

mkdir -p "$CLASS_DIR"

echo "Compiling Java benchmark..."
javac -d "$CLASS_DIR" "$SRC"

echo ""
echo "=== KIO vs Java: Simple Loop (1M) ==="

echo ""
echo "--- KIO ---"
"$KIO_BIN" examples/simple_benchmark_java.kio

echo ""
echo "--- Java ---"
java -cp "$CLASS_DIR" SimpleBenchmarkJava

