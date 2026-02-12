console.log("=== JavaScript Simple Benchmark ===");

const start = Date.now();

let sum = 0;
let i = 0;
while (i < 1000000) {
    sum = sum + i;
    i = i + 1;
}

const end = Date.now();
const elapsed = end - start;

console.log("Sum: " + sum);
console.log("Time: " + elapsed + " ms");
console.log("JavaScript is fast!");
