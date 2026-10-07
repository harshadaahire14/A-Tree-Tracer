# Tree-Walk Interpreter / Tracer in C++

A small educational programming-language interpreter written in C++17.

## Pipeline

Source Code -> Lexer -> Tokens -> Parser -> AST -> Tree-Walk Interpreter -> Output

The optional tracer prints AST execution events while the interpreter walks the tree.

## Supported language features

- Numbers, strings, booleans and nil
- Variables: `var x = 10;`
- Assignment: `x = x + 1;`
- Arithmetic: `+ - * /`
- Comparisons: `< <= > >= == !=`
- Logical operators: `and`, `or`, `!`
- `if / else`
- `while`
- Blocks `{ ... }`
- `print`
- `//` comments
- REPL
- Execution tracing with `--trace`

## Requirements

- C++17 compiler
- CMake 3.16+ (recommended)

## Build with CMake

```bash
cmake -S . -B build
cmake --build build
```

On Windows with Visual Studio:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Executable:
- Linux/macOS: `build/treewalk`
- Windows Visual Studio: `build/Release/treewalk.exe`

## Run

```bash
build/treewalk examples/demo.tw
```

Windows:

```powershell
.\build\Release\treewalk.exe examples\demo.tw
```

Tracing:

```powershell
.\build\Release\treewalk.exe --trace examples\demo.tw
```

REPL:

```powershell
.\build\Release\treewalk.exe
```

Inside REPL:

```text
>>> print(10 + 20);
30
>>> var x = 5;
>>> x = x * 2;
>>> print(x);
10
>>> :trace on
>>> print(x + 1);
```

## Project structure

```text
tree_walk_interpreter_cpp/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── ast.h
│   ├── environment.h
│   ├── interpreter.h
│   ├── lexer.h
│   ├── parser.h
│   ├── token.h
│   └── tracer.h
├── src/
│   ├── environment.cpp
│   ├── interpreter.cpp
│   ├── lexer.cpp
│   ├── main.cpp
│   ├── parser.cpp
│   ├── token_names.cpp
│   └── tracer.cpp
└── examples/
    └── demo.tw
```

## How execution works

1. The lexer reads characters and creates tokens.
2. The parser applies expression precedence rules and creates an AST.
3. The interpreter recursively evaluates AST nodes.
4. The environment stores variables and nested block scopes.
5. The tracer logs entry/exit events for AST nodes.

## Notes

This is intentionally a compact educational interpreter. It is a strong base for adding functions, return statements, arrays, a richer AST printer, break/continue, and a bytecode VM later.
