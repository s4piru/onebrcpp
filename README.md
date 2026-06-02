## Run profiler

```
brew install samply
clang++ -std=c++20 -O3 -Wall -Wextra -g -fno-omit-frame-pointer -o baseline baseline.cc
head measurements.txt -n 10000 > measurements100k.txt
samply record ./baseline measurements100k.txt
```