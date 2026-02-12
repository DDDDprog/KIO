console.log("--- JavaScript Benchmark Results ---");
const start = Date.now();
let i = 0;
let result = 0;

while (i < 1000000) {
    result = result + i * 2;
    if (result > 1000000) {
        result = result / 2;
    }
    i = i + 1;
}

const end = Date.now();
console.log("Iterations: 1,000,000");
console.log("Total Time: " + (end - start) + "ms");
console.log("Final Value: " + result);
