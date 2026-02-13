## KIO Language Reference (Snapshot 2026)

This document describes the **current KIO language** as implemented in the lexer and parser (`src/tokenizer/lexer.cpp`, `src/parser/parser.cpp`, `include/kio/ast.hpp`). It should be read together with the formal grammar in `docs/grammar/kio.ebnf`.

---

## Lexical structure

- **Identifiers**
  - Pattern: `[A-Za-z_][A-Za-z0-9_]*`
  - Examples: `foo`, `_tmp`, `HttpServer`, `x1`.
- **Numbers**
  - Decimal integers and decimals:
    - `123`, `0`, `42.0`, `3.14159`
  - Parsed as `NUMBER` tokens and stored as `double`.
- **Strings**
  - Normal strings: `"hello"`, `"line\nbreak"`, `"quote \" inside"`.
    - Support C-style escapes (`\n`, `\t`, `\r`, `\\`, `\"`, `\'`).
  - Raw strings: `r"no escapes here"`.
    - No escape processing; characters are taken as-is.
- **Character literals**
  - Single characters in single quotes: `'a'`, `'\n'`, `'\\'`.
  - Produced as `CHAR` tokens and currently treated as strings at the AST level.
- **Comments**
  - Line comments:
    - `// until end of line`
    - `# also used as line comment`
- **Whitespace**
  - Spaces, tabs, carriage returns, and newlines separate tokens but are otherwise ignored.

### Keywords

Recognized keywords in the current lexer include (non-exhaustive):

`print`, `let`, `save`, `load`, `sys`, `import`, `if`, `else`, `while`, `for`, `const`, `function`, `return`, `switch`, `case`, `default`, `break`, `continue`, `try`, `catch`, `throw`, `class`, `new`, `this`, `super`, `static`, `public`, `private`, `protected`, `interface`, `enum`, `namespace`, `using`, `as`, `is`, `async`, `await`, `yield`, `with`, `finally`, `from`, `module`, `export`, `parallel`, `in`, `true`, `false`, `nil`.

Not all of these have full parser or runtime support yet (e.g., `interface`, `enum`), but they are reserved and will be used by future language features.

---

## Program and modules

- A KIO **program** is a sequence of **declarations**:
  - Variable and constant declarations (`let`, `const`).
  - Function declarations (`function`).
  - Class declarations (`class`).
  - Namespaces (`namespace`).
  - Modules (`module`, `export`).
  - Top-level statements (e.g., `print`, expressions).
- The parser entry point is:
  - `program -> { declaration } EOF`

### Variables and constants

- **Mutable variable**:
  - `let x = 42;`
  - With type annotation:
    - `let x: number = 42;`
- **Constant**:
  - `const PI = 3.14159;`
  - With type annotation:
    - `const name: string = "kio";`
- `let` requires an initializer; if omitted today, the parser reports an error.

### Functions

Syntax (from `functionDeclaration`):

```kio
function name(param1: Type1, param2: Type2, ...) : ReturnType {
    // body
}
```

- Parameters:
  - Each parameter is `name` or `name: TypeName`.
  - Types are currently strings at the parser level; enforcement is handled by future type-system passes.
- Return type:
  - Optional `: TypeName` after the parameter list.
- Body:
  - A **block**: `{ ... }` containing declarations and statements.

### Namespaces and modules

- **Namespace**:

```kio
namespace math {
    let PI = 3.14159;
    function square(x: number): number { return x * x; }
}
```

- **Module**:

```kio
module db {
    export function connect(path: string) {
        // ...
    }
}
```

Internally, namespaces and modules both collect a list of nested `Stmt` nodes. The runtime/module system (`src/core/module_system.cpp`) is responsible for wiring these into loadable units.

### Imports and exports

- Simple import:

```kio
import "path/to/file.kio";
```

- Named import:

```kio
import Foo from "foo.kio";
```

- Exports:

```kio
export function foo() { ... }
export let answer = 42;
```

These constructs are parsed into AST nodes (`Stmt::Import`, `Stmt::ImportFrom`, `Stmt::Export`) and later consumed by the module system and/or compiler.

---

## Statements

### Blocks

```kio
{
    let x = 1;
    print x;
}
```

- A block introduces a new scope for local variables.
- Parsed as `Stmt::Block` with a list of nested `StmtPtr`.

### Print

```kio
print expression;
```

- Uses the VM or interpreter’s `valToString` logic to render values.

### Conditionals

```kio
if (condition) statement
else statement
```

- `condition` is any expression.
- `then` and `else` branches can be blocks or single statements.

### Loops

Two main forms are supported:

- **While loop**:

```kio
while (condition) {
    // body
}
```

- **C-style for loop**:

```kio
for (let i = 0; i < n; i = i + 1) {
    // body
}
```

- **For-in loop** (desugared over numeric ranges in the interpreter and compiled loops in the VM):

```kio
for i in range(10) {
    print i;
}
```

The parser recognizes `for i in expr statement` and encodes it as `Stmt::ForIn`.

### Switch

```kio
switch (expression) {
    case 1:
        print "one";
    case 2:
        print "two";
    default:
        print "other";
}
```

- Parsed as `Stmt::Switch` with:
  - A discriminant expression.
  - A list of `(caseExpr, caseStatements)` pairs.
  - Optional `defaultCase` statements.

### Try / catch / finally

```kio
try {
    // risky operations
} catch (e) {
    // error handling
} finally {
    // always executed
}
```

- Represented as `Stmt::TryCatch` with:
  - `tryBlock`, `catchVar`, `catchBlock`, `finallyBlock`.
- `throw expr;` creates a `Stmt::Throw`.

### Parallel blocks

```kio
parallel {
    // statements to run in parallel/executor context
}
```

- Parsed as `Stmt::Parallel` wrapping a list of statements.
- Execution strategy is defined by the runtime (`src/features/parallel_executor.cpp`).

### Save / load

```kio
save "state.kio";
load "state.kio";
```

- `save` and `load` accept a string literal path and are parsed as `Stmt::Save` / `Stmt::Load`.
- Runtime support can serialize/restore program state or data as needed.

### Control flow

- `break;` – exit the nearest loop or switch.
- `continue;` – skip to next loop iteration.
- `return expr;` – return from a function (value optional).

---

## Expressions

Expressions are parsed using a standard precedence hierarchy:

1. Assignment
2. Logical OR / AND
3. Equality
4. Comparison
5. Addition / subtraction
6. Multiplication / division / modulo
7. Unary / `sys` queries
8. Calls, indexing, and property access
9. Primary literals and variables

### Assignment

```kio
name = expr;
object.field = expr;
array[index] = expr;
```

- Simple `=` assignments are supported.
- Compound assignments (`+=`, `-=`, `*=`, `/=`) are parsed but their lowering is partially implemented; treat them as experimental.
- Targets can be:
  - Variables (`Expr::Variable`).
  - Object properties (`Expr::Get` → `Expr::Set`).
  - Array elements (`Expr::Index` → `Expr::IndexSet`).

### Logical expressions

```kio
a && b
a || b
```

- Logical operators short-circuit and are represented as `Expr::Logical`.

### Comparisons and arithmetic

```kio
a == b
a != b
a < b
a <= b
a > b
a >= b

a + b
a - b
a * b
a / b
a % b
```

- All of these are parsed into `Expr::Binary` nodes, using `TokenType` to distinguish the operator.

### Unary and post-increment/decrement

```kio
!expr
-expr
expr++
expr--
```

- Unary operators produce `Expr::Unary`.
- Postfix `++`/`--` on variables produce `Expr::PostOp`.

### Function calls, property access, and indexing

```kio
foo(1, 2, 3)
obj.method(arg)
array[0]
obj.nested.field
```

- Function calls: `Expr::Call`.
- Property access: `Expr::Get`.
- Indexing: `Expr::Index`.

### `sys` queries

```kio
sys "time"
sys("os_name")
```

- Parsed as `Expr::SysQuery` with a string key.
- Runtime maps keys like `"time"`, `"os_name"`, `"arch"`, `"kio_version"` to values (see `Interpreter` and `VM` implementations).

### Arrays

```kio
[1, 2, 3]
["a", "b", "c"]
```

- Array literals parse to `Expr::Array` with a list of nested expressions.
- VM bytecode uses `ARRAY_NEW`, `ARRAY_GET`, `ARRAY_SET` opcodes.

### Ternary operator (experimental)

```kio
condition ? thenExpr : elseExpr
```

- The parser defines a `ternary()` helper and an `Expr::Ternary` AST node.
- As of this snapshot, ternary is implemented but not fully wired as the top-level `expression` rule; treat it as an experimental feature.

---

## Types and annotations

- Type annotations are currently **syntactic only**:
  - `let x: number = 1;`
  - `function f(x: number): string { ... }`
- The parser records type strings in:
  - `Stmt::Var::typeAnnotation`
  - `Stmt::Function::params` and `Stmt::Function::returnType`
- A separate type system (`src/features/type_system.cpp`) is responsible for interpreting and enforcing these annotations; the core language grammar does not yet enforce types at parse time.

---

## Relationship to the C core

- The language as described here is **frontend-only**:
  - Lexer, parser, and AST are purely C++ today.
  - The C core exposes a VM and value ABI that execute bytecode compiled from these ASTs.
- As the C core evolves:
  - This document and `docs/grammar/kio.ebnf` will remain the source of truth for syntax.
  - New constructs (e.g., better modules, structured types, concurrency) will be added in sync with parser changes.

