# 1BRC C++ Implementation

## File Overview

- generate_input.sh: Generates a input file(measurements.txt) and expected output(expected_output.txt).
- baseline.cc: Simplest implementation.
- single_thread.cc: Optimized single-threaded implementation.
- multi_thread.cc: Multithreaded implementation.
- run_baseline.sh: Run baseline.cc.
- run_single_thread.sh: Run single_thread.cc.
- run_multi_thread: Run multi_thread.cc.

## Environment

- The optimized implementations use Unix-specific APIs. So the code is intended for Linux or macOS. Tested on Apple M2 Pro with 32 GB Memory.
- clang++ (for C++20) and Java for generate_input.sh


## Setup
### Get the original 1BRC repository

```
git submodule update --init --recursive
```

## Generate Input Data

```bash
./generate_input.sh
```

### Run

```bash
./run_baseline.sh
./run_single_thread.sh
./run_multi_thread.sh
```

- real time: wall-clock time
- user time: CPU time spent in user-space code
- sys time: CPU time spent in kernel/system calls

## Profiler (MacOS)

```
brew install samply
clang++ -std=c++20 -O3 -Wall -Wextra -g -fno-omit-frame-pointer -o baseline baseline.cc
head -n 100000 measurements.txt > measurements100k.txt
samply record ./baseline measurements100k.txt
```