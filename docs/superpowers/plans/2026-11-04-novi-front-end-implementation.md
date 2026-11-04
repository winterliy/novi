# Novi Front-End Syntax Implementation Plan

> **For agentic workers:** Track the implementation in small source-level changes; preserve the existing C++17 VM architecture.

**Goal:** Make the agreed Novi syntax executable in the C++ lexer, parser, compiler, and VM instead of documenting it only.

**Architecture:** Extend the existing token/AST/bytecode pipeline. Add whole-source input so brace blocks and declarations can span lines. Implement synchronous language constructs end to end; represent async syntax explicitly and keep scheduling semantics separate until a task runtime exists.

**Tech Stack:** C++17, current handwritten lexer, recursive-descent parser, bytecode compiler, stack VM.

**Spec:** `docs/tasks/novi-language-front-end.md`

## Global Constraints

- Preserve current REPL arithmetic and existing class/closure behavior.
- `=` is assignment; `==` is equality.
- `is`/`if` accept block and `then` single-statement forms.
- `reflex(value) target;` updates the target and rechecks the enclosing condition.
- Import/export statements end in `;`; only the built-in `{novi} from {novi}` import is available before a module loader exists.
- Class members default public; `#` and `private` mark private members; `static` marks class-owned members.
- Do not claim async scheduling until the VM has a task scheduler.

## Review Focus

- `=` inside a condition assigns; `==` compares.
- `reflex` must update the intended variable before re-evaluating the condition.
- Private access must be checked against the declaring class context.
- Import syntax must not silently pretend arbitrary user modules were loaded.
- Async syntax must not be described as concurrent while execution is synchronous.

## Implementation Order

1. Extend lexer tokens for identifiers, strings, punctuation, operators, and annotations.
2. Parse complete programs, declarations, expressions, blocks, `is/if then`, functions, lambdas, classes, and `reflex` loops.
3. Extend AST and bytecode for lambda/class fields/static/private metadata and `reflex` updates.
4. Wire runtime behavior for output, loops, functions, and class visibility; recognize built-in import/export syntax.
5. Add whole-file CLI input and retain the interactive REPL.
6. Update the language document to distinguish implemented syntax from future runtime work.
