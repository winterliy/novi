# Novi performance baseline

Build a Release executable, then run:

```sh
python3 benchmarks/source_lines.py build/novi --lines 10000 --runs 5
```

The script generates a temporary Novi program, checks its output on each run, and reports wall time for the complete process (startup, parsing, compilation and execution). Compare builds on the same machine and workload; this does not isolate VM execution.

2026-11-08 baseline on Apple Silicon arm64, Apple Clang 17, Release: 10,000 lines, 5 runs, median **0.253253 s**. CMake 4.4.5 built successfully; CTest passed (1 registered suite). This machine requires the SDK libc++ include path in `CMAKE_CXX_FLAGS`.
