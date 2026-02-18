import subprocess
import time
import os

def run_axeon():
    print("🚀 Running Axeon Benchmark...")
    # Using the local binary in dist/
    axeon_bin = "./dist/axeon"
    if not os.path.exists(axeon_bin):
        print("❌ Axeon binary not found at ./dist/axeon. Please build first.")
        return None
    
    start = time.time()
    result = subprocess.run([axeon_bin, "scripts/bench_axeon.axe"], capture_output=True, text=True)
    end = time.time()
    
    if result.returncode != 0:
        print("❌ Axeon benchmark failed:")
        print(result.stderr)
        return None
    
    print(result.stdout)
    # Extract time from output if available, otherwise use python's timer
    for line in result.stdout.splitlines():
        if "Time (ms):" in line:
            return float(line.split(":")[1].strip())
    return (end - start) * 1000

def run_js():
    print("🚀 Running JavaScript (Node.js) Benchmark...")
    start = time.time()
    result = subprocess.run(["node", "scripts/bench_js.js"], capture_output=True, text=True)
    end = time.time()
    
    if result.returncode != 0:
        print("❌ JavaScript benchmark failed. Is node installed?")
        return None
    
    print(result.stdout)
    for line in result.stdout.splitlines():
        if "Time (ms):" in line:
            return float(line.split(":")[1].strip())
    return (end - start) * 1000

if __name__ == "__main__":
    axeon_time = run_axeon()
    print("-" * 30)
    js_time = run_js()
    print("-" * 30)
    
    if axeon_time and js_time:
        print(f"📊 Comparison Results:")
        print(f"Axeon: {axeon_time:.2f} ms")
        print(f"Node.js: {js_time:.2f} ms")
        ratio = axeon_time / js_time
        if ratio < 1.0:
            print(f"🔥 Axeon is {1/ratio:.2f}x FASTER than Node.js!")
        else:
            print(f"🐢 Axeon is {ratio:.2f}x slower than Node.js")
