# Axeon Programming Language - Complete Documentation

**Version:** 2026

---

## Table of Contents

1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Language Basics](#language-basics)
4. [Data Types](#data-types)
5. [Operators](#operators)
6. [Control Flow](#control-flow)
7. [Functions](#functions)
8. [Classes](#classes)
9. [Modules](#modules)
10. [Arrays](#arrays)
11. [Strings](#strings)
12. [Exception Handling](#exception-handling)
13. [Parallel Execution](#parallel-execution)
14. [System Queries](#system-queries)
15. [Native Modules](#native-modules)
16. [Built-in Functions](#built-in-functions)
17. [Grammar Reference](#grammar-reference)
18. [Examples](#examples)

---

## 1. Introduction

Axeon is a high-performance, configurable programming language designed for speed and flexibility. It features a stack-based virtual machine with JIT compilation, garbage collection, and support for parallel execution.

### Key Features

- Minimal syntax with familiar constructs
- High-performance VM with computed gotos
- JIT (Just-In-Time) compilation support
- Mark-and-sweep garbage collection
- Native multi-threading with parallel blocks
- Cross-platform: Linux, macOS, Windows, iOS, Android, WASM
- SIMD vector support for heavy arithmetic
- Native modules: Crypto, GPU, GUI, Database, Network, Vector

### Platform Architecture

Axeon utilizes a High-Performance Stack-based Virtual Machine with:

- **Computed Gotos:** Optimized opcode dispatch for 30% faster execution
- **Mark-and-Sweep GC:** Efficient memory management for long-running processes
- **Parallel Executor:** Native multi-threading with `parallel` blocks
- **Platform Agnostic Core:** Deployable to macOS, Windows, Linux, iOS, Android, WASM
- **Vectorized Math:** SIMD support for heavy arithmetic via the `vector` module
- **JIT Compilation:** Tracing JIT for hot path optimization

---

## 2. Getting Started

### Installation

```bash
# Clone the repository
git clone https://github.com/axeon-lang/axeon.git
cd axeon

# Build
./build.sh

# Run
./axeon your_program.axe
```

### Your First Program

```axeon
print "Hello, World!";
```

### File Extensions

- **Source files:** `.axe`
- **Module files:** `.axe`
- **Saved state:** `.json`

---

## 3. Language Basics

### Variables

```axeon
let x = 42;
let name = "Axeon";
let pi = 3.14159;
```

**With type annotations:**
```axeon
let x: number = 42;
let name: string = "Axeon";
```

### Constants

```axeon
const MAX_SIZE = 100;
const PI_VALUE = 3.14159;
```

### Comments

```axeon
// This is a single-line comment

# This is also a single-line comment

/*
 * This is a
 * multi-line comment
 */
```

### Print Statement

```axeon
print "Hello, World!";
print "Value: " + 42;
println "This adds a newline";
```

---

## 4. Data Types

### Numbers

```axeon
let integer = 42;
let decimal = 3.14159;
let negative = -10;
```

### Strings

```axeon
let greeting = "Hello";
let with_escape = "Line one\nLine two";
let raw_string = r"No escapes here";
```

### Booleans

```axeon
let is_true = true;
let is_false = false;
```

### Nil

```axeon
let empty = nil;
```

### Arrays

```axeon
let numbers = [1, 2, 3, 4, 5];
let mixed = [1, "hello", true, 3.14];
```

---

## 5. Operators

### Arithmetic Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `a + b` |
| `-` | Subtraction | `a - b` |
| `*` | Multiplication | `a * b` |
| `/` | Division | `a / b` |
| `%` | Modulo | `a % b` |

### Comparison Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `==` | Equal to | `a == b` |
| `!=` | Not equal to | `a != b` |
| `<` | Less than | `a < b` |
| `<=` | Less than or equal | `a <= b` |
| `>` | Greater than | `a > b` |
| `>=` | Greater than or equal | `a >= b` |

### Logical Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `&&` | AND (short-circuit) | `a && b` |
| `||` | OR (short-circuit) | `a \|\| b` |
| `!` | NOT | `!a` |

### Assignment Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `=` | Simple assignment | `a = b` |
| `+=` | Add and assign | `a += b` |
| `-=` | Subtract and assign | `a -= b` |
| `*=` | Multiply and assign | `a *= b` |
| `/=` | Divide and assign | `a /= b` |

### Increment/Decrement

| Operator | Description | Example |
|----------|-------------|---------|
| `++` | Increment | `a++` |
| `--` | Decrement | `a--` |

---

## 6. Control Flow

### If-Else

```axeon
let score = 85;

if (score >= 90) {
    print "Grade: A";
} else if (score >= 80) {
    print "Grade: B";
} else if (score >= 70) {
    print "Grade: C";
} else {
    print "Grade: F";
}
```

### While Loop

```axeon
let count = 5;

while (count > 0) {
    print count;
    count = count - 1;
}

print "Blast off!";
```

### For Loop (C-style)

```axeon
for (let i = 0; i < 5; i = i + 1) {
    print i;
}
```

### For-In Loop

```axeon
for i in range(5) {
    print i;
}
```

### Switch Statement

```axeon
let value = 2;

switch (value) {
    case 1:
        print "one";
    case 2:
        print "two";
    default:
        print "other";
}
```

### Break and Continue

```axeon
for (let i = 0; i < 10; i = i + 1) {
    if (i == 3) {
        continue; // Skip iteration 3
    }
    if (i == 7) {
        break; // Exit loop at 7
    }
    print i;
}
```

---

## 7. Functions

### Function Declaration

```axeon
function greet(name) {
    return "Hello, " + name + "!";
}

print greet("Axeon");
```

### Function with Type Annotations

```axeon
function fib(n: number): number {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

print fib(10); // 55
```

### Function with Multiple Parameters

```axeon
function add(a, b, c) {
    return a + b + c;
}

print add(1, 2, 3); // 6
```

### Recursive Functions

```axeon
function factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

print factorial(5); // 120
```

---

## 8. Classes

### Basic Class

```axeon
class Animal {
    function speak() {
        print "...";
    }
    
    function walk() {
        print "Animal walks";
    }
}

let dog = Animal.new();
dog.speak();
```

### Class with Constructor

```axeon
class Person {
    function init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    function introduce() {
        print "I am " + this.name + ", " + this.age + " years old";
    }
}

let john = Person.new("John", 30);
john.introduce();
```

### Inheritance

```axeon
class Animal {
    function speak() {
        print "...";
    }
}

class Dog extends Animal {
    function speak() {
        print "Woof!";
    }
}

let dog = Dog.new();
dog.speak(); // "Woof!"
```

---

## 9. Modules

### Creating a Module

```axeon
// math.axe
module math {
    export function square(x) {
        return x * x;
    }
    
    export function cube(x) {
        return x * x * x;
    }
    
    const PI = 3.14159;
}
```

### Importing a Module

```axeon
import math from "math.axe";

print math.square(5);  // 25
print math.cube(2);    // 8
print math.PI;         // 3.14159
```

### Importing Everything

```axeon
import "constants.axe";

print PI;
print MAX_VALUE;
```

---

## 10. Arrays

### Array Literals

```axeon
let numbers = [1, 2, 3, 4, 5];
let mixed = [1, "hello", true, nil];
let empty = [];
```

### Array Access

```axeon
let fruits = ["apple", "banana", "cherry"];

print fruits[0];  // "apple"
print fruits[2]; // "cherry"
```

### Array Assignment

```axeon
let arr = [1, 2, 3];
arr[1] = 42;
print arr[1]; // 42
```

### Array Length

```axeon
let arr = [1, 2, 3, 4, 5];
print len(arr); // 5
```

### Array Iteration

```axeon
let items = ["a", "b", "c"];

for item in items {
    print item;
}
```

---

## 11. Strings

### String Concatenation

```axeon
let greeting = "Hello";
let target = "World";

let message = greeting + " " + target + "!";
print message; // "Hello World!"
```

### String Methods

```axeon
let text = "Hello, World!";

print upper(text);  // "HELLO, WORLD!"
print lower(text); // "hello, world!"
print len(text);   // 13
```

### Substring

```axeon
let text = "Hello, World!";
print substr(text, 0, 5);  // "Hello"
print substr(text, 7, 5);  // "World"
```

### Split and Join

```axeon
let parts = split("a,b,c", ",");
print parts; // ["a", "b", "c"]

let joined = join(["a", "b", "c"], "-");
print joined; // "a-b-c"
```

---

## 12. Exception Handling

### Try-Catch

```axeon
try {
    let result = risky_operation();
    print result;
} catch (e) {
    print "Error: " + e;
}
```

### Try-Catch-Finally

```axeon
try {
    print "Trying...";
} catch (e) {
    print "Caught: " + e;
} finally {
    print "Always runs";
}
```

### Throw

```axeon
function divide(a, b) {
    if (b == 0) {
        throw "Division by zero!";
    }
    return a / b;
}

try {
    print divide(10, 0);
} catch (e) {
    print e; // "Division by zero!"
}
```

---

## 13. Parallel Execution

```axeon
parallel {
    // These statements run in parallel
    let a = 0;
    for (let i = 0; i < 1000; i = i + 1) {
        a = a + 1;
    }
    print "Task 1: " + a;
}

parallel {
    let b = 0;
    for (let i = 0; i < 1000; i = i + 1) {
        b = b + 1;
    }
    print "Task 2: " + b;
}
```

---

## 14. System Queries

```axeon
print sys "os_name";      // Operating system name
print sys "cpu_model";    // CPU model
print sys "mem_total_kb"; // Total memory in KB
print sys "disk_root_kb";// Root disk space in KB
print sys "time";         // Current timestamp
print sys "kio_version"; // Axeon version
print sys "arch";        // Architecture
```

---

## 15. Native Modules

### Crypto Module

```axeon
let hash = crypto.sha256("password");
print hash;
```

### Database Module

```axeon
db.set("key1", "value1");
let val = db.get("key1");
print val;
```

### Vector Module

```axeon
let a = [1.0, 2.0, 3.0];
let b = [4.0, 5.0, 6.0];
let dot = vector.dot(a, b);
print dot;
```

### Network Module

```axeon
let server = net.server_init(8080);
let request = net.server_accept(server);
net.server_respond(server, request, 200, "text/plain", "Hello!");
```

### GUI Module

```axeon
let win = gui.window("My App", 800, 600);
gui.button(win, "Click Me", callback);
```

### BigInt Module

```axeon
let big = bigint.add("12345678901234567890", "98765432109876543210");
print big;
```

---

## 16. Built-in Functions

### Output Functions

| Function | Description |
|----------|-------------|
| `print` | Print to output |
| `println` | Print with newline |

### Type Conversion Functions

| Function | Description |
|----------|-------------|
| `str(value)` | Convert to string |
| `int_func(value)` | Convert to integer |
| `float_func(value)` | Convert to float |
| `bool_func(value)` | Convert to boolean |
| `type(value)` | Get type of value |
| `len(value)` | Get length of array or string |

### Math Functions

| Function | Description | Example |
|----------|-------------|---------|
| `abs_func(x)` | Absolute value | `abs_func(-5)` → 5 |
| `min_func(a, b)` | Minimum | `min_func(3, 7)` → 3 |
| `max_func(a, b)` | Maximum | `max_func(3, 7)` → 7 |
| `pow_func(a, b)` | Power | `pow_func(2, 3)` → 8 |
| `sqrt_func(x)` | Square root | `sqrt_func(16)` → 4 |

### String Functions

| Function | Description | Example |
|----------|-------------|---------|
| `upper(s)` | Uppercase | `upper("hello")` → "HELLO" |
| `lower(s)` | Lowercase | `lower("HELLO")` → "hello" |
| `substr(s, start, len)` | Substring | `substr("Hello", 0, 3)` → "Hel" |
| `split(s, delim)` | Split string | `split("a,b,c", ",")` → ["a","b","c"] |
| `join(arr, delim)` | Join array | `join(["a","b"], "-")` → "a-b" |

---

## 17. Grammar Reference

### Lexical Structure

- **Identifiers**: `[a-zA-Z_][a-zA-Z0-9_]*`
- **Numbers**: `[0-9]+(\.[0-9]*)?`
- **Strings**: `"[^"]*"`
- **Comments**: `// ...` or `/* ... */`

### Keywords

`let`, `if`, `else`, `while`, `for`, `in`, `function`, `return`, `class`, `super`, `this`, `parallel`, `import`, `export`, `module`, `sys`, `true`, `false`, `nil`, `try`, `catch`, `finally`, `throw`, `switch`, `case`, `default`, `break`, `continue`, `const`, `new`, `namespace`.

### Statements Grammar

```
program         -> declaration* EOF ;
declaration     -> varDecl | funcDecl | classDecl | statement ;
varDecl         -> "let" IDENTIFIER ( "=" expression )? ";" ;
constDecl       -> "const" IDENTIFIER ( "=" expression )? ";" ;
funcDecl        -> "function" IDENTIFIER "(" params? ")" block ;
classDecl       -> "class" IDENTIFIER ( "extends" IDENTIFIER )? "{" declaration* "}" ;
statement       -> exprStmt | ifStmt | whileStmt | forStmt | switchStmt | returnStmt | breakStmt | continueStmt | tryStmt | throwStmt | parallelStmt | importStmt | block ;
block           -> "{" declaration* "}" ;
```

### Expressions Grammar

```
expression      -> assignment ;
assignment      -> ( call "." )? IDENTIFIER "=" assignment | ternary ;
ternary         -> logical_or ( "?" expression ":" expression )? ;
logical_or      -> logical_and ( "||" logical_and )* ;
logical_and     -> equality ( "&&" equality )* ;
equality        -> comparison ( ( "!=" | "==" ) comparison )* ;
comparison      -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term            -> factor ( ( "-" | "+" ) factor )* ;
factor          -> unary ( ( "/" | "*" | "%" ) unary )* ;
unary           -> ( "!" | "-" ) unary | postfix ;
postfix         -> call ( "++" | "--" )? ;
call            -> primary ( "(" arguments? ")" | "." IDENTIFIER | "[" expression "]" )* ;
primary         -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" | IDENTIFIER | array ;
array           -> "[" ( expression ( "," expression )* )? "]" ;
arguments       -> expression ( "," expression )* ;
params          -> param ( "," param )* ;
param           -> IDENTIFIER ( ":" IDENTIFIER )? ;
```

---

## 18. Examples

### Hello World

```axeon
print "Hello, World!";
```

### Fibonacci Sequence

```axeon
let n = 10;
let a = 0;
let b = 1;

print "First " + n + " Fibonacci numbers:";

let i = 0;
while (i < n) {
    print a;
    let next = a + b;
    a = b;
    b = next;
    i = i + 1;
}
```

### FizzBuzz

```axeon
let n = 1;

while (n <= 20) {
    let fizz = n % 3 == 0;
    let buzz = n % 5 == 0;
    
    if (fizz && buzz) {
        print "FizzBuzz";
    } else if (fizz) {
        print "Fizz";
    } else if (buzz) {
        print "Buzz";
    } else {
        print n;
    }
    
    n = n + 1;
}
```

### Save/Load State

```axeon
let name = "John";
let age = 30;
let score = 98.5;

save "state.json";
print "State saved!";

load "state.json";
print "State loaded!";
```

### Scope Example

```axeon
let x = "global";

print "Start: " + x;

{
    let x = "inner";
    print "Inside block: " + x;
    
    {
        let y = "deepest";
        print "Deepest: " + y + ", x: " + x;
    }
}

print "Back to global: " + x;
```

### Advanced Features Demo

```axeon
print "=== Axeon Advanced Demo ===";

// Type annotations
let:num count = 5;
let:str name = "Axeon";

print "Count: " + count;
print "Name: " + name;

// Compound assignment
let total = 10;
total += 5;
print "Total: " + total;

// Ternary operator
let age = 20;
let status = age >= 18 ? "adult" : "minor";
print "Status: " + status;
```

---

*Axeon Programming Language - Complete Documentation*
*Version 2026*
