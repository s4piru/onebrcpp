#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cassert>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <algorithm>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

struct StringHash {
  using is_transparent = void;
  size_t operator()(std::string_view sv) const {
    return std::hash<std::string_view>{}(sv);
  }
};

struct AlwaysEqual {
  using is_transparent = void;
  template <typename T, typename U>
  bool operator()(const T&, const U&) const { return true; }
};

struct Stats {
  double min = std::numeric_limits<double>::infinity();
  double max = -std::numeric_limits<double>::infinity();
  double sum = 0;
  double count = 0;
};

using Map = std::unordered_map<std::string, Stats, StringHash, AlwaysEqual>;

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

std::pair<std::string_view, double> ParseLine(const char** p) {
  const char* start = *p;
  while (**p != ';') ++(*p);
  std::string_view name(start, *p - start);
  ++(*p);
  double temp = ParseDouble(p);
  ++(*p);
  return {name, temp};
}

// Takes a pointer to data, start offset and end offset, and returns a map.
// Skips until the beginning of the line unless it's the first chunk.
// Handles until the end of the last line which might be after the end of the chunk.
Map ParseChunk(const char* data, size_t start, size_t end, size_t file_size) {
  const char* p = data + start;
  const char* file_end = data + file_size;

  if (start != 0) {
    while (p < file_end && *(p - 1) != '\n') ++p;
  }

  const char* chunk_end = data + end;
  if (end < file_size) {
    while (chunk_end < file_end && *(chunk_end - 1) != '\n') ++chunk_end;
  }

  Map ma;
  while (p < chunk_end) {
    auto [name, tmp] = ParseLine(&p);
    auto it = ma.find(name);
    if (it == ma.end()) {
      it = ma.emplace(std::string(name), Stats{}).first;
    }
    Stats& st = it->second;
    st.min = std::min(st.min, tmp);
    st.max = std::max(st.max, tmp);
    st.sum += tmp;
    ++st.count;
  }

  return ma;
}

// Takes a vector of maps and returns a map.
Map Merge(const std::vector<Map>& maps) {
  Map result;
  for (const auto& ma : maps) {
    for (const auto& [name, st] : ma) {
      auto it = result.find(name);
      if (it == result.end()) {
        result.emplace(name, st);
      } else {
        Stats& r = it->second;
        r.min = std::min(r.min, st.min);
        r.max = std::max(r.max, st.max);
        r.sum += st.sum;
        r.count += st.count;
      }
    }
  }
  return result;
}

void Print(const Map& ma) {
  std::vector<std::string> keys;
  keys.reserve(ma.size());
  for (const auto& [name, _] : ma) {
    keys.push_back(name);
  }
  std::sort(keys.begin(), keys.end());

  std::cout << std::fixed << std::setprecision(1);
  std::cout << "{";
  bool first = true;
  for (const auto& name : keys) {
    const Stats& st = ma.find(name)->second;
    if (!first) {
      std::cout << ", ";
    }
    first = false;
    const double mean = st.sum / st.count;
    std::cout << name << "=" << st.min << "/" << mean << "/" << st.max;
  }
  std::cout << "}" << std::endl;
}

int main(int argc, char* argv[]) {
  assert(argc >= 2);

  int fd = open(argv[1], O_RDONLY);
  struct stat sb;
  fstat(fd, &sb);
  const char* data =
      static_cast<const char*>(mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));

  // Create as many futures as the number of cores.
  unsigned int num_threads = std::thread::hardware_concurrency();
  if (num_threads == 0) num_threads = 1;
  const size_t chunk_size = sb.st_size / num_threads;

  // Split the file into chunks, each thread gets its own map.
  std::vector<std::future<Map>> futures;
  for (unsigned int i = 0; i < num_threads; ++i) {
    size_t start = i * chunk_size;
    size_t end = (i == num_threads - 1) ? sb.st_size : (i + 1) * chunk_size;
    futures.push_back(std::async(std::launch::async, ParseChunk, data, start, end, sb.st_size));
  }

  std::vector<Map> maps;
  maps.reserve(num_threads);
  for (auto& f : futures) {
    maps.push_back(f.get());
  }

  // Merge at the end.
  Map ma = Merge(maps);
  Print(ma);

  munmap(const_cast<char*>(data), sb.st_size);
  close(fd);

  return 0;
}
