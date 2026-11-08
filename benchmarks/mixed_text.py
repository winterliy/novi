#!/usr/bin/env python3
"""Measure repeated length calls on a mixed ASCII/Korean string."""
import argparse
from pathlib import Path
import statistics
import subprocess
import tempfile
import time

parser = argparse.ArgumentParser()
parser.add_argument('executable', type=Path)
parser.add_argument('--runs', type=int, default=5)
args = parser.parse_args()
if args.runs < 1:
    parser.error('runs must be positive')
with tempfile.TemporaryDirectory(prefix='novi-mixed-text-') as directory:
    source = Path(directory) / 'mixed.nvi'
    source.write_text("vari text : string = '" + '한a' * 500 + "';\n"
                      "vari n : integer = 0;\n"
                      "during (n < 30000) { vari length : integer = text.length(); n = n + 1; }\n"
                      "console.print(n:string);\n", encoding='utf-8')
    durations = []
    for _ in range(args.runs):
        start = time.perf_counter()
        result = subprocess.run([str(args.executable.resolve()), str(source)],
                                capture_output=True, text=True, check=True)
        durations.append(time.perf_counter() - start)
        if result.stdout != '30000\n':
            raise RuntimeError(f'unexpected output: {result.stdout!r}')
print(f'runs={args.runs} median_seconds={statistics.median(durations):.6f}')
print('samples_seconds=' + ','.join(f'{value:.6f}' for value in durations))
