# Novi Object-Oriented Backend Implementation Plan

> **For agentic workers:** Implement this plan task-by-task in this session. User explicitly assigned native implementation. Steps are implementation checkpoints; do not add or run tests unless the user asks.

**Goal:** Build a C++17 compiler and bytecode interpreter foundation for Novi, including an object-oriented runtime that can execute classes, instances, methods, fields, and inheritance independently of the eventual surface syntax.

**Architecture:** Keep syntax in a small parser boundary and compile a syntax-neutral AST into stack bytecode. Execute bytecode in a VM with tagged values, lexical environments, call frames, classes, instances, bound methods, and source-aware runtime errors. The first CLI parser only supports the calculator syntax already described in README; OOP syntax remains for the user to design.

**Tech Stack:** C++17, standard library only, Apple Clang/VS Code task already used by the repository.

**Spec:** `docs/tasks/novi-object-oriented-backend.md`

## Global Constraints

- Do not settle Novi's class, function, declaration, or import spelling in the parser.
- Preserve source locations through parse, compile, and runtime execution.
- Keep backend APIs independent of parser token spellings.
- Use C++17 and avoid external runtime dependencies.
- Do not add or run tests unless the user asks for tests or verification.

## Review Focus

- Runtime errors should retain the Novi source location that produced them.
- Method calls must bind their receiver while preserving lexical scope.
- Inheritance lookup must terminate and resolve nearest definitions first.
- Division by zero and invalid operand types must report language errors, not C++ undefined behavior.
- Recursive or deeply nested calls must fail with a controlled VM error rather than corrupting the value stack.

---

### Task 1: Source diagnostics and syntax-neutral AST

**Files:**
- Create: `src/novi/source.hpp`
- Create: `src/novi/ast.hpp`
- Create: `src/novi/diagnostic.hpp`

**Interfaces:**
- `SourceLocation { file, offset, line, column }`
- `SourceSpan { start, end }`
- `Diagnostic { category, message, span }`
- AST nodes for literals, variables, assignment, unary/binary expressions, calls, property get/set, blocks, variable declarations, conditionals, loops, functions, returns, classes, and expression statements.

- [ ] Define these data structures with source spans on every node.
- [ ] Keep AST node names semantic; do not encode surface token names in node types.

### Task 2: Values and object model

**Files:**
- Create: `src/novi/value.hpp`
- Create: `src/novi/runtime.cpp`

**Interfaces:**
- `Value` variant for null, boolean, number, string, native callable, function, class, instance, and bound method.
- `Environment` with lexical parent and named slots.
- `ClassObject` with optional parent and method table.
- `InstanceObject` with class reference and instance fields.
- `RuntimeError` carrying a `SourceSpan` and call stack locations.

- [ ] Implement type-safe value formatting and equality.
- [ ] Implement lexical name lookup/assignment and nearest-parent class method lookup.
- [ ] Implement bound-method receiver capture and class instantiation semantics.

### Task 3: Bytecode representation and compiler

**Files:**
- Create: `src/novi/bytecode.hpp`
- Create: `src/novi/compiler.hpp`
- Create: `src/novi/compiler.cpp`

**Interfaces:**
- `Instruction { OpCode op, int operand, SourceSpan span }`
- `Chunk { code, constants, nested functions }`
- `Compiler::compile(const Program&) -> CompiledModule`

- [ ] Add constants, names, stack operations, math/comparison operations, jumps, calls/returns, closures, classes, properties, and method binding opcodes.
- [ ] Compile AST control flow with patched jump targets.
- [ ] Compile functions and class methods into nested function prototypes.
- [ ] Reject invalid AST/compile state with source-aware diagnostics.

### Task 4: VM and interpreter

**Files:**
- Create: `src/novi/vm.hpp`
- Create: `src/novi/vm.cpp`

**Interfaces:**
- `VM::run(const CompiledModule&) -> Value`
- Call frames hold function, instruction pointer, locals/environment, and receiver binding.
- VM owns operand stack, global environment, and native function registry.

- [ ] Execute values, arithmetic, branches, variables, calls, returns, and closures.
- [ ] Execute class definition, instance construction, field access, methods, and inheritance dispatch.
- [ ] Convert runtime failures into source-aware `RuntimeError` call stacks.
- [ ] Guard stack/frame bounds and invalid bytecode operands.

### Task 5: Minimal calculator parser and REPL bridge

**Files:**
- Create: `src/novi/lexer.hpp`
- Create: `src/novi/lexer.cpp`
- Create: `src/novi/parser.hpp`
- Create: `src/novi/parser.cpp`
- Modify: `src/main.cpp`

**Interfaces:**
- `Lexer::scan(source) -> tokens + diagnostics`
- `Parser::parse(tokens) -> Program + diagnostics`
- `main` reads one line at a time, parses, compiles, and runs it.

- [ ] Support only the documented calculator surface: numbers, `+ - * /`, parentheses, and line-oriented input.
- [ ] Lower parsed calculator expressions to syntax-neutral AST and use the compiler/VM path.
- [ ] Keep declarations, function/class keywords, and import spelling out until the user specifies them.
- [ ] Update README to describe the implemented pipeline and build command.

### Task 6: Build integration

**Files:**
- Modify: `.vscode/tasks.json`
- Create: `CMakeLists.txt` (optional if it remains simpler than expanding the existing task)

- [ ] Compile all Novi sources under C++17.
- [ ] Keep the existing VS Code build/run workflow usable.
- [ ] Confirm the production sources compile; do not run tests unless requested.

