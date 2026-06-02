#!/bin/bash
set -e

clang++ -std=c++20 -O3 -Wall -Wextra -o baseline baseline.cc

time ./baseline measurements.txt > /tmp/baseline_output.txt

if diff -q expected_output.txt /tmp/baseline_output.txt >/dev/null 2>&1; then
  printf '\033[32mPASS\033[0m\n'
else
  printf '\033[31mFAIL\033[0m\n'
  exit 1
fi
