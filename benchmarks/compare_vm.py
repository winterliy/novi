#!/usr/bin/env python3
"""Compare two novi_vm_benchmark executables using alternating runs."""
import argparse
from pathlib import Path
import statistics
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('before', type=Path)
parser.add_argument('after', type=Path)
parser.add_argument('--runs', type=int, default=15)
args = parser.parse_args()
if args.runs < 1:
    parser.error('runs must be positive')
paths = {'before': args.before.resolve(), 'after': args.after.resolve()}
samples = {name: [] for name in paths}
for iteration in range(args.runs):
    order = ('before', 'after') if iteration % 2 == 0 else ('after', 'before')
    for name in order:
        output = subprocess.check_output([str(paths[name])], text=True).strip()
        key, value = output.split('=', 1)
        if key != 'vm_seconds':
            raise RuntimeError(f'unexpected output: {output!r}')
        samples[name].append(float(value))
for name, values in samples.items():
    print(f'{name} median_seconds={statistics.median(values):.6f} '
          f'min_seconds={min(values):.6f} max_seconds={max(values):.6f}')
