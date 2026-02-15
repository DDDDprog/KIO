const { performance } = require('perf_hooks');

let sum = 0;
const start = performance.now();
const iterations = 10000000;

for (let i = 0; i < iterations; i++) {
    sum = sum + i * 2;
    sum = sum - i;
}

const end = performance.now();
console.log("--- JavaScript Benchmark Results ---");
console.log("Iterations: " + iterations);
console.log("Final Sum: " + sum);
console.log("Total Time (ms): " + (end - start));
