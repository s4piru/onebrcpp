#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>

struct Stats {
  double min = std::numeric_limits<double>::infinity();
  double max = -std::numeric_limits<double>::infinity();
  double sum = 0;
  double count = 0;
};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }
  std::ifstream file(argv[1]);

  std::map<std::string, Stats> ma;

  std::string line;
  while (std::getline(file, line)) {
    const auto sep = line.find(';');
    const std::string name = line.substr(0, sep);

    std::istringstream ss(line.substr(sep + 1));

    double tmp = 0.0;
    ss >> tmp;

    Stats& st = ma[name];
    st.min = std::min(st.min, tmp);
    st.max = std::max(st.max, tmp);
    st.sum += tmp;
    ++st.count;
  }

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
