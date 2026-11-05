잠정 중단

# Novi

Novi is an experimental C/C++ language implementation focused on optimizing the programs it compiles. C++ is used for the compiler, VM, and object runtime; C is reserved for system APIs and stable native interfaces. Assembly may be added only for measured CPU-specific bottlenecks, with a C++ fallback. Novi-language source files use the `.nvi` extension. The compiler currently connects basic expressions, declarations, functions, conditionals, loops, and class/method syntax to the bytecode VM; some type, access-control, and module semantics are still incomplete.

## Build

The project requires a C++17 compiler and CMake 3.16 or newer:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
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
./build/novi                 # interactive REPL
./build/novi examples/hello.nvi
./build/novi new my-project
```

Enter expressions such as `1 + 2 * 3`; type `exit` to leave the REPL. A `.nvi` file is parsed as one complete program, so declarations and brace blocks can span lines.

`novi new my-project` creates a starter project with `src/main.nvi`, `novi.toml`, `README.md`, and `.gitignore`. Run the generated entry file with `novi my-project/src/main.nvi`.

## Design tasks

- [Novi front-end syntax and feature decisions](docs/tasks/novi-language-front-end.md)
- [Object-oriented compiler and interpreter backend](docs/tasks/novi-object-oriented-backend.md)
