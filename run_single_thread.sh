#!/bin/bash
set -e

clang++ -std=c++20 -O3 -Wall -Wextra -o single_thread single_thread.cc

time ./single_thread measurements.txt > /tmp/single_thread_output.txt

if diff -q expected_output.txt /tmp/single_thread_output.txt >/dev/null 2>&1; then
  printf '\033[32mPASS\033[0m\n'
else
  printf '\033[31mFAIL\033[0m\n'
  exit 1
fi
