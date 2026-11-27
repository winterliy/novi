# Novi Mathematical Core Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a useful, shape-aware mathematical core to Novi: explicit numeric domains, dense homogeneous N-dimensional tensors, checked shape rules, and core scalar/tensor operations.

**Architecture:** Keep mathematical values separate from general-purpose lists and from symbolic expressions. Add a typed numerical/tensor layer behind the current Value/VM, with runtime shape checks first and static shape inference as a later compiler improvement. Keep differentiation, symbolic algebra, sparse solvers, and proof checking as separate follow-up subsystems.

**Tech Stack:** C++17, existing Novi lexer/parser/AST/bytecode/VM; no mandatory third-party runtime dependency in the first milestone.

**Spec:** `docs/tasks/novi-mathematics-design.md`

## Global Constraints

- Novi source files use `.nvi`; implementation files remain C++ `.cpp`/`.hpp`.
- Array indexing starts at zero.
- Mathematical tensors contain homogeneous elements with an explicit dtype and runtime shape.
- Do not conflate `null`, a failed lookup, an unknown proposition, `nan`, or an undefined name.
- Shape mismatch and invalid numeric operations produce typed Novi runtime errors with source spans.
- Optimizations must preserve the selected numeric type and documented accuracy semantics.
- Do not fix tensor syntax until the user chooses the proposed frontend forms; library/runtime APIs may be developed independently.

## Review Focus

- Empty shapes, rank-zero tensors, and zero-length axes have documented behavior and never cause invalid indexing.
- Broadcasting follows one written rule and rejects incompatible shapes with both operand shapes in the diagnostic.
- Matrix multiplication checks inner dimensions and returns the correct output shape.
- Numeric promotion is deterministic, symmetric where mathematically appropriate, and never silently narrows exact values.
- Equality and logical operators distinguish scalar booleans from boolean tensors/masks.
- Allocation size multiplication is overflow-checked before memory allocation.

## Scope

This plan delivers the first runnable math milestone: scalar numeric values plus dense tensors, shape-aware operations, and a stable C++ interface. The roadmap after this milestone is: (1) linear algebra and sparse formats, (2) exact rational/decimal and richer complex arithmetic, (3) numerical integration/optimization/statistics, (4) autodiff/vectorization/JIT, (5) symbolic expressions, (6) proposition/proof tooling. Each later subsystem gets its own plan after the core interface is reviewed.

## Accepted Frontend Defaults

The user accepted this direction and these defaults. Remaining fixed-shape and contraction details are still open:

```novi
vari values : tensor<realn> = tensor([1, 2, 3]);
vari grid : tensor<realn, [2, 3]> = tensor([[1, 2, 3], [4, 5, 6]]);
vari product : tensor<realn> = matmul(left, right);
vari elementwise : tensor<realn> = left .* right;
vari slope : realn = grad(function {x} then output(x * x))(3);
```

Proposed defaults to decide:

- Runtime shape is the default; optional fixed extents may be used for static checks and specialization.
- `vector`/`matrix` are rank-1/rank-2 aliases of `tensor`.
- `+`/`-` are elementwise; `.*` and `./` are explicit elementwise multiply/divide; named `matmul()` is the unambiguous linear algebra operation. If the user prefers `*` as matrix product, scalar multiplication remains allowed and rank rules must be stated.
- Dense row-major layout is the first implementation; layout is internal and not observable in semantics.
- Broadcasting only occurs under the standard trailing-axis rule: paired extents are compatible when equal or either is 1.
- Boolean tensor masks use elementwise mask operations and stay distinct from scalar `and`/`or` and mathematical propositions.
- The existing unusual split (`&`/`|` logical, `&&`/`||` bitwise) remains untouched until the user decides how those operators act on tensor masks.

## Planned File Map

- Create `src/novi/math/numeric.hpp/.cpp`: numeric kinds, promotion, checked scalar operations, and runtime numeric errors.
- Create `src/novi/math/tensor.hpp/.cpp`: dtype-tagged dense tensor storage, shape, row-major strides, construction, indexing, reshape, and broadcasting validation.
- Create `src/novi/math/ops.hpp/.cpp`: elementwise arithmetic, reductions, and matrix multiplication with a generic dispatch boundary.
- Modify `src/novi/value.hpp`, `src/novi/runtime.cpp`, and `src/novi/vm.cpp`: store and print tensors as Novi values, implement equality/truthiness policy, and expose math built-ins.
- Modify `src/novi/bytecode.hpp` and `src/novi/compiler.cpp`: emit the small set of math built-in calls after syntax is approved.
- Modify `src/novi/lexer.cpp`, `src/novi/parser.cpp`, and `src/novi/ast.hpp` only after syntax decisions are approved.
- Modify `CMakeLists.txt` to compile the new implementation units.
- Create `tests/novi/math/numeric_tests.cpp`, `tensor_tests.cpp`, and `operation_tests.cpp`; add them to CTest when implementation begins.

## Tasks

### Task 1: Define numeric semantics and error types

**Files:** Create `src/novi/math/numeric.hpp/.cpp`; create `tests/novi/math/numeric_tests.cpp`.

- [ ] Define `NumericKind` for `int64`, `float32`, `float64`, and `complex128` as the first supported storage kinds; document exact/approximate semantics and checked overflow.
- [ ] Implement symmetric `promote_numeric_kind(left, right)` and checked arithmetic helpers.
- [ ] Add tests for kind promotion, integer overflow, division by zero, and rejected lossy conversion.

### Task 2: Add homogeneous dense tensor storage

**Files:** Create `src/novi/math/tensor.hpp/.cpp`; create `tests/novi/math/tensor_tests.cpp`.

- [ ] Implement `Tensor` with `NumericKind`, `std::vector<std::size_t> shape`, checked element count, contiguous row-major values, and source-independent storage.
- [ ] Implement zero-based multi-index flattening with bounds/rank checks; support scalar rank zero and zero-length dimensions.
- [ ] Implement construction from flat values and nested rectangular lists; reject ragged nested input.
- [ ] Implement shape-preserving `reshape` and metadata queries `rank`, `shape`, and `size`.
- [ ] Add tests for rank 0–N, 0-based indexing, bounds errors, empty dimensions, ragged input, and allocation-overflow guards.

### Task 3: Implement shape rules and tensor operations

**Files:** Create `src/novi/math/ops.hpp/.cpp`; create `tests/novi/math/operation_tests.cpp`.

- [ ] Implement a single trailing-axis broadcasting shape function and return a diagnostic for incompatible axes.
- [ ] Implement elementwise `add`, `subtract`, `multiply`, and `divide` with scalar promotion and broadcasted shapes.
- [ ] Implement `sum` over all elements and an explicit-axis reduction interface.
- [ ] Implement 1D/2D `matmul` first, including vector dot product, matrix-vector, and matrix-matrix cases; reject unsupported ranks explicitly.
- [ ] Add tests for scalar broadcast, vector/matrix shapes, invalid broadcast, integer/float promotion, and each matmul rank case.

### Task 4: Integrate math values with the Novi VM

**Files:** Modify `src/novi/value.hpp`, `src/novi/runtime.cpp`, `src/novi/vm.cpp`, `src/novi/bytecode.hpp`, `src/novi/compiler.cpp`, `CMakeLists.txt`; create `tests/novi/math/vm_math_tests.cpp`.

- [ ] Add an owning tensor value alternative to `Value`; ensure copies share immutable storage or perform explicit value semantics consistently.
- [ ] Define Novi formatting for tensors with dtype, shape, and compact element output.
- [ ] Define scalar truth conversion to reject tensors; require explicit reductions such as `all(mask)` or `any(mask)` for conditions.
- [ ] Expose the approved `tensor`, `shape`, `rank`, `sum`, and `matmul` entry points as built-ins.
- [ ] Add end-to-end `.nvi` programs to verify creation, operations, output, and source-located errors.

### Task 5: Approve syntax and connect tensor AST forms

**Files:** Modify `docs/tasks/novi-mathematics-design.md`, `docs/tasks/novi-language-front-end.md`, `src/novi/lexer.cpp`, `src/novi/parser.cpp`, `src/novi/ast.hpp`, `src/novi/compiler.cpp`; create parser/compiler tests.

- [ ] Confirm the optional fixed-shape spelling and whether the contraction helper is named `contract` or `einsum`.
- [ ] Record the accepted tensor forms in the frontend document.
- [ ] Add parser/AST forms and lower them to the math built-ins without duplicating shape or numeric semantics in the compiler.
- [ ] Add syntax-to-runtime integration tests for the accepted `.nvi` examples.

## Later Separate Plans

- **Exact arithmetic:** arbitrary-precision integer, normalized rational, exact decimal, mixed promotion, complex precision kinds.
- **Scientific library:** decompositions/eigenvalues, sparse matrices, integration/ODE, optimization, statistics, signal processing.
- **Autodiff and vectorization:** derivative rules on pure math IR, `grad`/Jacobian/Hessian, `vmap`, then behavior-preserving `jit`.
- **Symbolic algebra:** symbol assumptions, exact expression DAG, simplify/substitute/differentiate/integrate/solve.
- **Logic and proof:** proposition AST, `unknown` semantics, proof objects, then optional external solver/kernel integration.
