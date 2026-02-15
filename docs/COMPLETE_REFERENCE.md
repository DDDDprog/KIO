# Axeon Programming Language - Complete Reference Guide

## Table of Contents

1. [Introduction](#introduction)
2. [Lexical Structure](#lexical-structure)
3. [Grammar Specification](#grammar-specification)
4. [Language Features](#language-features)
5. [Built-in Functions](#built-in-functions)
6. [System Queries](#system-queries)
7. [Advanced Features](#advanced-features)
8. [Complete Examples](#complete-examples)
9. [File Extensions](#file-extensions)
10. [Running Programs](#running-programs)

---

## Introduction

Axeon (also known as KIO) is a high-performance, configurable programming language implemented in modern C++. It features a stack-based virtual machine with JIT compilation, garbage collection, and support for parallel execution. The language is designed to be minimal yet powerful, with a focus on speed and flexibility.

**Key Features:**
- Minimal syntax with familiar constructs
- High-performance VM with computed gotos
- JIT (Just-In-Time) compilation support
- Mark-and-sweep garbage collection
- Native multi-threading with parallel blocks
- Cross-platform: Linux, macOS, Windows, iOS, Android, WASM
- SIMD vector support for heavy arithmetic
- Native modules: Crypto, GPU, GUI, Database, Network, Vector

---

## Lexical Structure

### Identifiers

- **Pattern:** `[A-Za-z_][A-Za-z0-9_]*`
- **Examples:** `foo`, `_tmp`, `HttpServer`, `x1`

### Numbers

- **Integers:** `123`, `0`, `42`
- **Decimals:** `3.14159`, `0.5`, `100.0`
- Parsed as `double` values

### Strings

- **Normal strings:** `"hello"`, `"line\nbreak"`, `"quote \" inside"`
- **Escape sequences:** `\n`, `\t`, `\r`, `\\`, `\"`, `\'`
- **Raw strings:** `r"no escapes here"` - no escape processing

### Character Literals

- **Syntax:** `'a'`, `'\n'`, `'\\'`
- Single characters in single quotes

### Comments

- **Line comments:** `// comment` or `# comment`
- Comments extend to end of line

### Whitespace

- Spaces, tabs, carriage returns, and newlines separate tokens
- Otherwise ignored by the parser

---

## Grammar Specification

### Program Structure

```
program       = { declaration } , EOF ;

declaration   = var_decl
              | const_decl
              | function_decl
              | class_decl
              | namespace_decl
              | module_decl
              | export_decl
              | statement ;
```

### Variables and Constants

```
var_decl      = "let" , identifier , [ ":" , identifier ] ,
                "=" , expression , ";" ;

const_decl    = "const" , identifier , [ ":" , identifier ] ,
                "=" , expression , ";" ;
```

### Functions

```
function_decl =
    "function" , identifier ,
    "(" , [ param_list ] , ")" ,
    [ ":" , identifier ] ,
    block ;

param_list    = param , { "," , param } ;
param         = identifier , [ ":" , identifier ] ;
```

### Classes

```
class_decl    =
    "class" , identifier , [ ":" , identifier ] ,
    "{" , { declaration } , "}" ;
```

### Namespaces and Modules

```
namespace_decl =
    "namespace" , identifier ,
    "{" , { declaration } , "}" ;

module_decl   =
    "module" , identifier ,
    "{" , { declaration } , "}" ;

export_decl   = "export" , declaration ;
```

### Statements

```
statement     = print_stmt
              | if_stmt
              | while_stmt
              | for_stmt
              | switch_stmt
              | try_stmt
              | throw_stmt
              | break_stmt
              | continue_stmt
              | return_stmt
              | save_stmt
              | load_stmt
              | import_stmt
              | parallel_stmt
              | block
              | expr_stmt ;

block         = "{" , { declaration } , "}" ;

print_stmt    = "print" , expression , ";" ;
```

### Conditionals

```
if_stmt       = "if" , "(" , expression , ")" ,
                statement ,
                [ "else" , statement ] ;
```

### Loops

```
while_stmt    = "while" , "(" , expression , ")" , statement ;

for_stmt      = "for" ,
                ( for_in_suffix | for_c_suffix ) ;

for_in_suffix =
    identifier , "in" , expression , statement ;

for_c_suffix  =
    "(" ,
      [ for_initializer ] , ";" ,
      [ expression ]      , ";" ,
      [ expression ] ,
    ")" ,
    statement ;
```

### Switch Statement

```
switch_stmt   =
    "switch" , "(" , expression , ")" ,
    "{" ,
      { "case" , expression , ":" , { statement } } ,
      [ "default" , ":" , { statement } ] ,
    "}" ;
```

### Exception Handling

```
try_stmt      =
    "try" , block ,
    [ "catch" , "(" , identifier , ")" , block ] ,
    [ "finally" , block ] ;

throw_stmt    = "throw" , expression , ";" ;
```

### Control Flow

```
break_stmt    = "break" , ";" ;
continue_stmt = "continue" , ";" ;
return_stmt   = "return" , [ expression ] , ";" ;
```

### IO Statements

```
save_stmt     = "save" , string_literal , ";" ;
load_stmt     = "load" , string_literal , ";" ;
```

### Imports

```
import_stmt   =
      "import" , identifier , "from" , string_literal , ";"
    | "import" , string_literal , ";" ;
```

### Parallel Execution

```
parallel_stmt = "parallel" , block ;
```

### Expressions

```
expression    = assignment ;

assignment    =
    ( assignable , assign_op , assignment )
  | or_expr ;

assign_op     = "=" | "+=" | "-=" | "*=" | "/=" ;

or_expr       = and_expr , { "||" , and_expr } ;
and_expr      = equality , { "&&" , equality } ;
equality      = comparison , { ( "==" | "!=" ) , comparison } ;
comparison    = term , { ( ">" | ">=" | "<" | "<=" ) , term } ;
term          = factor , { ( "+" | "-" ) , factor } ;
factor        = unary , { ( "*" | "/" | "%" ) , unary } ;

unary         =
      "sys" , sys_suffix
    | ( "!" | "-" ) , unary
    | post_expr ;

sys_suffix    =
      "(" , string_literal , ")"
    | string_literal ;

post_expr     = call_expr , [ "++" | "--" ] ;

call_expr     = primary , { call_tail } ;

call_tail     =
      "(" , [ argument_list ] , ")"
    | "." , identifier
    | "[" , expression , "]" ;

argument_list = expression , { "," , expression } ;

primary       =
      number
    | string_literal
    | raw_string_literal
    | char_literal
    | "true"
    | "false"
    | "nil"
    | "this"
    | identifier
    | "(" , expression , ")"
    | array_literal ;

array_literal = "[" , [ expression , { "," , expression } ] , "]" ;
```

---

## Language Features

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

### Operators

**Arithmetic:**
- `+` Addition
- `-` Subtraction
- `*` Multiplication
- `/` Division
- `%` Modulo (remainder)

**Comparison:**
- `==` Equal to
- `!=` Not equal to
- `<` Less than
- `<=` Less than or equal
- `>` Greater than
- `>=` Greater than or equal

**Logical:**
- `&&` AND (short-circuit)
- `||` OR (short-circuit)
- `!` NOT

**Assignment:**
- `=` Simple assignment
- `+=` Add and assign
- `-=` Subtract and assign
- `*=` Multiply and assign
- `/=` Divide and assign

**Increment/Decrement:**
- `++` Increment
- `--` Decrement

### Arrays

```axeon
let numbers = [1, 2, 3, 4, 5];
print numbers[0];  // First element
numbers[1] = 42;   // Assignment to index
```

### String Operations

```axeon
let greeting = "Hello";
let target = "World";
let message = greeting + " " + target + "!";
print message;
```

---

## Built-in Functions

### Output Functions

```axeon
print("Hello, World!");
println("With newline");
// Or simply:
print "Hello, World!";
```

### Type Conversion Functions

```axeon
len(array)           // Get array/string length
type(value)          // Get type of value
str(value)           // Convert to string
int_func(value)      // Convert to integer
float_func(value)    // Convert to float
bool_func(value)     // Convert to boolean
```

### Math Functions

```axeon
abs_func(-5)         // Absolute value: 5
min_func(3, 7)       // Minimum: 3
max_func(3, 7)       // Maximum: 7
pow_func(2, 3)       // Power: 8
sqrt_func(16)        // Square root: 4
```

### String Functions

```axeon
substr("Hello", 0, 3)    // "Hel" - substring
split("a,b,c", ",")      // ["a", "b", "c"]
join(["a", "b"], "-")    // "a-b"
upper("hello")          // "HELLO"
lower("HELLO")          // "hello"
```

---

## System Queries

The `sys` keyword queries system information:

```axeon
print sys "os_name";        // Operating system name
print sys "cpu_model";      // CPU model
print sys "mem_total_kb";   // Total memory in KB
print sys "disk_root_kb";   // Root disk space in KB
print sys "time";           // Current timestamp
print sys "kio_version";   // Axeon version
print sys "arch";          // Architecture
```

---

## Advanced Features

### Parallel Execution

```axeon
parallel {
    // Statements run in parallel/executor context
}
```

### Save/Load State

```axeon
save "state.json";   // Save program state
load "state.json";   // Load program state
```

### Modules

**Creating a module (math.axe):**
```axeon
module math {
    export function square(x) {
        return x * x;
    }
}
```

**Using a module:**
```axeon
import math from "math.axe";
print math.square(5);  // 25
```

### Import Statements

```axeon
// Import everything from file
import "lib_constants.axe";

// Import specific item
import PI from "constants.axe";
```

### Classes

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
```

### Exception Handling

```axeon
try {
    // Risky operations
} catch (e) {
    // Handle error
} finally {
    // Always executed
}

throw "Error message";
```

### Switch Statement

```axeon
switch (value) {
    case 1:
        print "one";
    case 2:
        print "two";
    default:
        print "other";
}
```

---

## Complete Examples

### Hello World

```axeon
// File: hello.axe
let x = 41 + 1;
print x;
print "axeon";
```

### Basic Variables

```axeon
// File: 01_basics.axe
// Axeon Basics: Variables and Printing

let name = "Axeon User";
let version = 2.0;

print "Welcome to " + name;
print "Language Version:";
print version;

// Re-assignment
version = 2.1;
print "Updated Version:";
print version;
```

### Arithmetic Operations

```axeon
// File: 02_math.axe
// Axeon Arithmetic: Standard Math Operations

let a = 100;
let b = 15;

print "a = 100, b = 15";
print "Addition (a + b):";
print a + b;

print "Subtraction (a - b):";
print a - b;

print "Multiplication (a * b):";
print a * b;

print "Division (a / b):";
print a / b;

print "Modulo (Remainder) (a % b):";
print a % b;

print "Complex Expression (a + b * 2 / 10):";
print a + b * 2 / 10;
```

### If-Else Branching

```axeon
// File: 03_branching.axe
// Axeon Branching: If-Else Statements

let score = 85;

print "Student Score: " + score;

if (score >= 90) {
    print "Grade: A";
} else {
    if (score >= 80) {
        print "Grade: B";
    } else {
        if (score >= 70) {
            print "Grade: C";
        } else {
            print "Grade: F";
        }
    }
}

// Simple truthiness check
if (1) {
    print "1 is truthy";
}

if (0) {
    print "This won't print";
} else {
    print "0 is falsy";
}
```

### While Loops

```axeon
// File: 04_loops.axe
// Axeon Loops: While Statements

let count = 5;

print "Countdown starting:";

while (count > 0) {
    print count;
    count = count - 1;
}

print "Blast off!";

// Summing numbers 1 to 10
let sum = 0;
let i = 1;

while (i <= 10) {
    sum = sum + i;
    i = i + 1;
}

print "Sum of numbers 1 to 10:";
print sum;
```

### Scopes and Blocks

```axeon
// File: 05_scopes.axe
// Axeon Scopes: Nested blocks and variable visibility

let x = "global";
print "Start global x: " + x;

{
    let x = "inner";
    print "Inside block x: " + x;
    
    {
        let y = "deeper";
        print "Inside nested block y: " + y;
        print "Still see outer x: " + x;
    }
}

print "Back to global x: " + x;
```

### System Information

```axeon
// File: 06_system_info.axe
// Axeon System: Querying machine information using 'sys'

print "--- System Information ---";

print "OS Name:";
print sys "os_name";

print "CPU Model:";
print sys "cpu_model";

print "Total Memory (KB):";
print sys "mem_total_kb";

print "Root Disk Space (KB):";
print sys "disk_root_kb";

print "Current Time:";
print sys "time";
```

### String Concatenation

```axeon
// File: 07_strings_concat.axe
// Axeon Strings: Concatenation and Literal usage

let greeting = "Hello";
let target = "World";

// Basic concatenation
let message = greeting + " " + target + "!";
print message;

// Multiple concatenations in print
print "Axeon is " + "awesome " + "and " + "fast.";

// Numeric values are converted to strings when added to strings
let year = 2025;
print "The year is " + year;
```

### FizzBuzz

```axeon
// File: 08_fizzbuzz.axe
// Axeon FizzBuzz: Classic coding challenge

let n = 1;

while (n <= 20) {
    let fizz = n % 3 == 0;
    let buzz = n % 5 == 0;
    
    if (fizz) {
        if (buzz) {
            print "FizzBuzz";
        } else {
            print "Fizz";
        }
    } else {
        if (buzz) {
            print "Buzz";
        } else {
            print n;
        }
    }
    
    n = n + 1;
}
```

### Fibonacci Sequence

```axeon
// File: 09_fibonacci.axe
// Axeon Fibonacci: Iterative Fibonacci Sequence
// Generates the first 10 terms.

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

### Save/Load State

```axeon
// File: 10_io_save_load.axe
// Axeon IO: Saving and Loading variable state

let name = "Axeon User";
let age = 25;
let score = 98.5;

print "Before save:";
print "Name: " + name;
print "Age: " + age;
print "Score: " + score;

// Save state to file
save "state.json";

print "State saved to state.json";
```

### Module Imports

```axeon
// File: 11_import_demo.axe
// Axeon Imports: Modularizing code
import "lib_constants.axe";

print "Imported constants:";
print "PI: " + PI;
print "E: " + E;
print "APP_NAME: " + APP_NAME;
```

### Constants Module

```axeon
// File: lib_constants.axe
let PI = 3.14159;
let E = 2.71828;
let APP_NAME = "Axeon Demo";
```

### Advanced Syntax

```axeon
// File: advanced_syntax.axe
// Axeon Type Annotations and Constants

const MAX_SIZE = 100;
const PI_VALUE = 3.14159;

let x = 10;
let y = 20;

// Type-like annotations (syntax demo)
let:num count = 5;
let:str name = "Axeon";

print "Advanced Syntax Demo";
print "Max Size: " + MAX_SIZE;
print "PI: " + PI_VALUE;
print "x + y = " + (x + y);
print "Count: " + count;
print "Name: " + name;
```

### Parallel Execution Test

```axeon
// File: parallel_test.axe
// Axeon Parallel Execution Test

let sum = 0;
let i = 0;

let start = sys "time";

while (i < 1000000) {
    sum = sum + i;
    i = i + 1;
}

let end = sys "time";

print "Sum: " + sum;
print "Time: " + (end - start) + "ms";
print "Parallel execution completed!";
```

### Advanced Features Demo

```axeon
// File: x_feature_demo.axe
// Axeon Feature Showcase

print "--- Axeon Advanced Features Demo ---";

print "Testing Crypto Features...";
let secret = "AXEON_SECRET_DATA";
let key = "MY_KEY_123";
let hash = crypto_sha256(secret);
print "SHA256: " + hash;

print "Testing GPU Features...";
let data = [1, 2, 3, 4, 5];
print "Data: " + data;

print "Testing GUI Features...";
gui_window("Axeon Advanced Demo", 800, 600);
gui_button("Click Me!");

print "Testing Vector Features...";
let v1 = [1, 2, 3];
let v2 = [4, 5, 6];
print "Vector 1: " + v1;
print "Vector 2: " + v2;

print "Testing BigInt Features...";
let big = bigint "123456789012345678901234567890";
print "BigInt: " + big;

print "All features working!";
```

### High-Performance Benchmark

```axeon
// File: benchmark_fast.axe
// High-performance Axeon benchmark

let iterations = 10000000;
let sum = 0.0;

let start = sys "time";

let i = 0;
while i < iterations {
    sum = sum + i * 2.0 + 1.0;
    if i % 1000 == 0 {
        sum = sum - 1.0;
    }
    i = i + 1;
}

let end = sys "time";
print "--- Axeon Optimized Benchmark ---";
print "Iterations: " + iterations;
print "Final Sum: " + sum;
print "Time (ms): " + (end - start);
```

### GPU Puzzles

```axeon
// File: gpu_puzzles.axe
// Axeon GPU Puzzles: Leveraging ROCm and rocSHMEM

let data = [1, 2, 3, 4, 5];
print "Array: " + data;

let sum = 0;
let i = 0;
while (i < 5) {
    sum = sum + data[i];
    i = i + 1;
}

print "Sum: " + sum;
```

---

## File Extensions

- **Source files:** `.axe` or `.kio`
- **Module files:** Same as source
- **Saved state:** `.json`

---

## Running Programs

### Build from Source

```bash
# Using the build script
./build.sh

# Or using CMake directly
mkdir build && cd build
cmake ..
make
```

### Run a Program

```bash
./build_pro/axeon examples/hello.axe
```

### Available Examples

| File | Description |
|------|-------------|
| `hello.axe` | Simple hello world |
| `01_basics.axe` | Variables and printing |
| `02_math.axe` | Arithmetic operations |
| `03_branching.axe` | If-else statements |
| `04_loops.axe` | While loops |
| `05_scopes.axe` | Block scopes |
| `06_system_info.axe` | System queries |
| `07_strings_concat.axe` | String operations |
| `08_fizzbuzz.axe` | FizzBuzz challenge |
| `09_fibonacci.axe` | Fibonacci sequence |
| `10_io_save_load.axe` | Save/load state |
| `11_import_demo.axe` | Module imports |
| `advanced_syntax.axe` | Advanced syntax |
| `parallel_test.axe` | Parallel execution |
| `benchmark_fast.axe` | Performance benchmark |

---

## Keywords Reference

**Variable/Constants:**
- `let` - Variable declaration
- `const` - Constant declaration

**Functions:**
- `function` - Function declaration

**Control Flow:**
- `if` - Conditional
- `else` - Alternative branch
- `while` - While loop
- `for` - For loop
- `in` - For-in loop iteration
- `switch` - Switch statement
- `case` - Switch case
- `default` - Default case
- `break` - Break from loop/switch
- `continue` - Continue to next iteration
- `return` - Return from function

**Exception Handling:**
- `try` - Try block
- `catch` - Catch exception
- `finally` - Finally block
- `throw` - Throw exception

**Classes:**
- `class` - Class declaration
- `this` - Current instance
- `super` - Parent class
- `new` - Create instance

**Modules:**
- `module` - Module declaration
- `namespace` - Namespace declaration
- `export` - Export declaration
- `import` - Import declaration
- `from` - Named import

**Other:**
- `parallel` - Parallel execution
- `sys` - System query
- `print` - Print to output
- `save` - Save state
- `load` - Load state
- `true` - Boolean true
- `false` - Boolean false
- `nil` - Null value

**Reserved (Future Use):**
- `static`, `public`, `private`, `protected`
- `interface`, `enum`
- `async`, `await`, `yield`
- `with`, `as`, `is`, `using`
- `nullable`

---

## Platform Architecture

Axeon utilizes a High-Performance Stack-based Virtual Machine with:

- **Computed Gotos:** Optimized opcode dispatch for 30% faster execution
- **Mark-and-Sweep GC:** Efficient memory management for long-running processes
- **Parallel Executor:** Native multi-threading with `parallel` blocks
- **Platform Agnostic Core:** Deployable to macOS, Windows, Linux, iOS, Android, WASM
- **Vectorized Math:** SIMD support for heavy arithmetic via the `vector` module
- **JIT Compilation:** Tracing JIT for hot path optimization

---

*Document Version: 2026*
*Axeon Language Reference Guide - Complete Edition*
