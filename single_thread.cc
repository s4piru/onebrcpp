#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cassert>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <string>

struct Stats {
  double min = std::numeric_limits<double>::infinity();
  double max = -std::numeric_limits<double>::infinity();
  double sum = 0;
  double count = 0;
};

double ParseDouble(const char** p) {
  bool negative = false;
  if (**p == '-') {
    negative = true;
    ++(*p);
  }
  double result = 0;
  while (**p != '.' && **p != '\n') {
    result *= 10.0;
    result += static_cast<double>(**p - '0');
    ++(*p);
  }
  if (**p == '.') {
    ++(*p);
    result += static_cast<double>(**p - '0') / 10.0;
    ++(*p);
  }
  return negative ? -result : result;
}

std::pair<std::string, double> ParseLine(const char** p) {
  const char* start = *p;
  while (**p != ';') ++(*p);
  std::string name(start, *p - start);
  assert(**p == ';');
  ++(*p);
  double temp = ParseDouble(p);
  assert(**p == '\n');
  ++(*p);
  return {name, temp};
}

int main(int argc, char* argv[]) {
  assert(argc >= 2);

  int fd = open(argv[1], O_RDONLY);
  struct stat sb;
  fstat(fd, &sb);
  const char* data =
      static_cast<const char*>(mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));

  std::map<std::string, Stats> ma;
  const char* p = data;
  const char* end = data + sb.st_size;
  while (p < end) {
    auto [name, tmp] = ParseLine(&p);
    Stats& st = ma[name];
    st.min = std::min(st.min, tmp);
    st.max = std::max(st.max, tmp);
    st.sum += tmp;
    ++st.count;
  }

  munmap(const_cast<char*>(data), sb.st_size);
  close(fd);

  std::cout << std::fixed << std::setprecision(1);
  std::cout << "{";
  bool first = true;
  for (const auto& [name, st] : ma) {
    if (!first) {
      std::cout << ", ";
    }
    first = false;
    const double mean = st.sum / st.count;
    std::cout << name << "=" << st.min << "/" << mean << "/" << st.max;
  }
  std::cout << "}" << std::endl;

  return 0;
}
