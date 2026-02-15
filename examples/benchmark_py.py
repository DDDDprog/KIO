import time

def benchmark():
    sum_val = 0
    iterations = 10000000
    start = time.time()
    
    i = 0
    while i < iterations:
        sum_val = sum_val + i * 2
        sum_val = sum_val - i
        i = i + 1
        
    end = time.time()
    print("--- Python Benchmark Results ---")
    print(f"Iterations: {iterations}")
    print(f"Final Sum: {sum_val}")
    print(f"Total Time (ms): {(end - start) * 1000:.2f}")

if __name__ == "__main__":
    benchmark()
