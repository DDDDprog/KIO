console.log("=== JavaScript Vectorization Benchmark ===");
const start = Date.now();

let x = 1.1;
let y = 2.2;
let z = 3.3;
let result = 0.0;
let i = 0;

while (i < 50000000) {
    result = result + (x * y) - (z / x);
    x = x + 0.00001;
    i = i + 1;
}

const end = Date.now();
console.log("Iterations: 50,000,000");
console.log("Values: " + x + ", " + result);
console.log("Time: " + (end - start) + "ms");
