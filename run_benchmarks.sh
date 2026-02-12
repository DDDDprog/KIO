#!/bin/bash
export KIO_BIN="./build/kio"

echo "=== BENCHMARK SUITE ==="

function run_comparative_benchmark() {
    TEST_NAME=$1
    KIO_FILE=$2
    JS_FILE=$3
    
    echo ""
    echo "--- $TEST_NAME ---"
    echo "Running KIO..."
    $KIO_BIN $KIO_FILE 
    echo ""
    echo "Running Node.js..."
    node $JS_FILE
}

run_comparative_benchmark "Simple Loop (1M)" "examples/simple_benchmark.kio" "examples/simple_benchmark.js"
run_comparative_benchmark "Calculation & Condition (1M)" "examples/benchmark.kio" "examples/benchmark.js"
run_comparative_benchmark "Float Math (5M)" "examples/tracing_jit_benchmark.kio" "examples/tracing_jit_benchmark.js"
