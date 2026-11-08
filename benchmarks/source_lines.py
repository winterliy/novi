#!/usr/bin/env python3
"""Measure end-to-end processing of a large Novi source file."""
import argparse
import pathlib
import statistics
import subprocess
import tempfile
import time

parser = argparse.ArgumentParser()
parser.add_argument('executable', type=pathlib.Path)
parser.add_argument('--lines', type=int, default=10000)
parser.add_argument('--runs', type=int, default=5)
args = parser.parse_args()
if args.lines < 1 or args.runs < 1:
    parser.error('lines and runs must be positive')
executable = args.executable.resolve()
with tempfile.TemporaryDirectory(prefix='novi-benchmark-') as directory:
    source = pathlib.Path(directory) / 'lines.nvi'
    source.write_text(''.join(f'vari v{i} : integer = {i};\n' for i in range(args.lines)) +
                      f'console.print(v{args.lines - 1}:string);\n', encoding='utf-8')
    durations = []
    for _ in range(args.runs):
        start = time.perf_counter()
        result = subprocess.run([str(executable), str(source)], capture_output=True, text=True, check=True)
        durations.append(time.perf_counter() - start)
        if result.stdout.strip() != str(args.lines - 1):
            raise RuntimeError(f'unexpected output: {result.stdout!r}')
print(f'lines={args.lines} runs={args.runs} median_seconds={statistics.median(durations):.6f}')
print('samples_seconds=' + ','.join(f'{value:.6f}' for value in durations))
