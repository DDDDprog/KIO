## KIO by Example (Snapshot 2026)

This file contains small, self-contained examples that exercise the current KIO language and runtime. They are aligned with the grammar in `docs/grammar/kio.ebnf` and the implementation in `src/`.

---

### Hello world

```kio
print "hello, kio!";
```

---

### Variables and arithmetic

```kio
let x: number = 10;
let y: number = 20;
let sum: number = x + y;

print sum; // 30
```

---

### Functions and return values

```kio
function fib(n: number): number {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

print fib(10); // 55
```

---

### For loop variants

```kio
// C-style for loop
for (let i = 0; i < 5; i = i + 1) {
    print i;
}

// For-in over a range
for i in range(5) {
    print i;
}
```

---

### Arrays and indexing

```kio
let xs = [1, 2, 3, 4];
print xs[0]; // 1

xs[1] = 42;
print xs[1]; // 42
```

---

### Modules and imports

```kio
// math.kio
module math {
    export function square(x: number): number {
        return x * x;
    }
}
```

```kio
// main.kio
import math from "math.kio";

print math.square(5); // 25
```

---

### Using native modules (crypto, db, net, vector)

> The exact module names and bindings may vary; this is an illustrative example
> of how native modules are intended to be used.

```kio
// Crypto hashing
let hash = crypto.sha256("hello");
print hash;

// Key/value database
db.set("answer", "42");
print db.get("answer");

// Vector math (assuming bindings into src/vector/vector_ops.cpp)
let a = [1.0, 2.0, 3.0];
let b = [4.0, 5.0, 6.0];
let dot = vector.dot(a, b);
print dot;
```

