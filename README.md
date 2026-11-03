# Novi

Novi is an experimental C++ language implementation. Its backend foundation includes source spans and diagnostics, a syntax-neutral AST, bytecode compilation, and a stack-based interpreter VM with function and class runtime objects. The surface parser is deliberately limited to calculator expressions until the Novi syntax decisions in [`docs/tasks/novi-language-front-end.md`](docs/tasks/novi-language-front-end.md) are filled in.

## Build

The project requires a C++17 compiler and CMake 3.16 or newer:

```sh
cmake -S . -B build
cmake --build build
```

Or compile directly with a C++17 compiler:

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic -Isrc \
  src/main.cpp src/novi/compiler.cpp src/novi/lexer.cpp \
  src/novi/parser.cpp src/novi/runtime.cpp src/novi/vm.cpp \
  -o novi
```

## Run

```sh
./build/novi
```

Enter arithmetic expressions such as `1 + 2 * 3`; type `exit` to leave the REPL. The object/function bytecode model is available to compiler-generated modules, but class/function surface syntax and a resolver/type checker remain future work pending language decisions.

## Design tasks

- [Novi front-end syntax and feature decisions](docs/tasks/novi-language-front-end.md)
- [Object-oriented compiler and interpreter backend](docs/tasks/novi-object-oriented-backend.md)
