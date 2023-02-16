#! /usr/bin/env python3

import os
import sys
import shutil


def print_usage(argv0):
    print(f'Usage {argv0} BENCH_DATA_PATH (fs mounted)')


if len(sys.argv) != 2:
    print_usage(sys.argv[0])
    sys.exit(1)

BENCH_PATH = sys.argv[1]
print(f'Will set path to: {BENCH_PATH}')

for fname in os.listdir('.'):
    if fname.endswith('.f'):
        tmp_fname = f'{fname}.tmp'
        HAS_SET = False
        with open(fname) as fr:
            with open(tmp_fname, 'w') as fw:
                for line in fr:
                    if not HAS_SET and line.strip().startswith('set'):
                        if not HAS_SET:
                            fw.write(f'set $dir={BENCH_PATH}\n')
                        HAS_SET = True
                    if 'set $dir' in line and 'dirwidth' not in line:
                        continue
                    fw.write(line)
        shutil.move(tmp_fname, fname)
