const { performance } = require('perf_hooks');

const iterations = 100000000;
let sum = 0;

const start = performance.now();

for (let i = 0; i < iterations; i++) {
    sum = sum + i * 2 - Math.floor(i / 2) + (i % 3);
}

const end = performance.now();

console.log("--- JavaScript Benchmark Results ---");
console.log("Iterations: " + iterations);
console.log("Final Sum: " + sum);
console.log("Time (ms): " + (end - start).toFixed(2));
