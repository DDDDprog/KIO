# Axeon Language Grammar 2.0 (2026 Edition)

## Lexical Structure

- **Identifiers**: `[a-zA-Z_][a-zA-Z0-9_]*`
- **Numbers**: `[0-9]+(\.[0-9]*)?`
- **Strings**: `"[^"]*"`
- **Comments**: `// ...` or `/* ... */`

## Keywords

`let`, `if`, `else`, `while`, `for`, `in`, `fn`, `return`, `class`, `super`, `this`, `parallel`, `import`, `export`, `module`, `sys`, `true`, `false`, `nil`, `try`, `catch`, `finally`, `throw`, `switch`, `case`, `default`, `break`, `continue`.

## Syntax

### Statements

```
program    -> declaration* EOF ;
declaration -> varDecl | funcDecl | classDecl | statement ;
varDecl     -> "let" IDENTIFIER ( "=" expression )? ";" ;
statement   -> exprStmt | ifStmt | whileStmt | forStmt | block | returnStmt | parallelStmt | importStmt ;
block       -> "{" declaration* "}" ;
```

### Expressions

```
expression -> assignment ;
assignment -> ( call "." )? IDENTIFIER "=" assignment | ternary ;
ternary    -> logical_or ( "?" expression ":" expression )? ;
logical_or -> logical_and ( "or" logical_and )* ;
logical_and -> equality ( "and" equality )* ;
equality   -> comparison ( ( "!=" | "==" ) comparison )* ;
comparison -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term       -> factor ( ( "-" | "+" ) factor )* ;
factor     -> unary ( ( "/" | "*" ) unary )* ;
unary      -> ( "!" | "-" ) unary | postop ;
postop     -> call ( "++" | "--" )? ;
call       -> primary ( "(" arguments? ")" | "." IDENTIFIER | "[" expression "]" )* ;
primary    -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" | IDENTIFIER | array ;
array      -> "[" ( expression ( "," expression )* )? "]" ;
```

## Platform Architecture

 Axeon a High-Performance Stack-based Virtual Machine with:
- **Computed Gotos**: Optimized opcode dispatch for 30% faster execution.
- **Mark-and-Sweep GC**: Efficient memory management for long-running processes.
- **Parallel Executor**: Native multi-threading with `parallel` blocks.
- **Platform Agnostic Core**: Deployable to macOS, Windows, Linux, iOS, Android, WASM.
- **Vectorized Math**: SIMD support for heavy arithmetic via the `vector` module.
