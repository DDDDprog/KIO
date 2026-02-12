import os
import subprocess
import time

KIO_BIN = "./build/kio"
EXAMPLES_DIR = "examples"

def run_example(filename):
    filepath = os.path.join(EXAMPLES_DIR, filename)
    print(f"Running {filename}...", end=" ", flush=True)
    
    start_time = time.time()
    try:
        result = subprocess.run([KIO_BIN, filepath], capture_output=True, text=True, timeout=5)
        duration = time.time() - start_time
        
        if result.returncode == 0:
            print(f"✅ Passed ({duration:.2f}s)")
            return True, result.stdout
        else:
            print(f"❌ Failed (Return Code: {result.returncode})")
            print("--- Stderr ---")
            print(result.stderr)
            return False, result.stderr
    except subprocess.TimeoutExpired:
        print("❌ Timed Out")
        return False, "Timeout"
    except Exception as e:
        print(f"❌ Error: {e}")
        return False, str(e)

def main():
    if not os.path.exists(KIO_BIN):
        print(f"Error: KIO binary not found at {KIO_BIN}")
        return

    files = [f for f in os.listdir(EXAMPLES_DIR) if f.endswith('.kio')]
    files.sort()
    
    passed = 0
    total = len(files)
    
    for f in files:
        success, _ = run_example(f)
        if success:
            passed += 1
            
    print(f"\nSummary: {passed}/{total} tests passed.")

if __name__ == "__main__":
    main()
