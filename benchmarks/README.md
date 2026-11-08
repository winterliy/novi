# Novi performance baseline

Build a Release executable, then run:

```sh
python3 benchmarks/source_lines.py build/novi --lines 10000 --runs 5
```

The script generates a temporary Novi program, checks its output on each run, and reports wall time for the complete process (startup, parsing, compilation and execution). Compare builds on the same machine and workload; this does not isolate VM execution.

2026-11-08 baseline on Apple Silicon arm64, Apple Clang 17, Release: 10,000 lines, 5 runs, median **0.253253 s**. CMake 4.4.5 built successfully; CTest passed (1 registered suite). This machine requires the SDK libc++ include path in `CMAKE_CXX_FLAGS`.

After switching source-line lookup from a linear scan to binary search, the same build settings and workload produced a 5-run median of **0.025641 s** (about 9.9× faster end to end for this generated long-file workload). This is not a general runtime speedup claim. The source location boundary test and the existing CTest suite passed after the change.

## VM loop

Configure with `-DNOVI_BUILD_BENCHMARKS=ON`, then run `build/novi_vm_benchmark` repeatedly. It compiles once before timing and measures `VM::run` for a 100,000-step range and sum, checking the printed result. VM setup is outside the timer; execution includes output and its own initialization. On the same Apple Clang 17 Release setup, the first run took 0.027031 seconds. Use repeated runs on the same machine for comparisons.
