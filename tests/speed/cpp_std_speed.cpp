#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>

double test_speed(std::regex &re, size_t n) {
  std::string test_string(n, 'a');

  std::smatch match;
  auto start_time = std::chrono::high_resolution_clock::now();
  bool found = std::regex_match(test_string, match, re);
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                      end_time - start_time)
                      .count();
  return duration;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <tests> <rounds>\n";
    return 1;
  }
  size_t tests = std::stoul(argv[1]);
  size_t rounds = std::stoul(argv[2]);
  for (size_t i = 0; i < tests; ++i) {
    double total_time = 0.0;
    std::string test_pattern = "";
    for (size_t j = 0; j < i + 1; ++j) {
      test_pattern += "a?";
    }
    test_pattern += std::string(i + 1, 'a');
    std::regex re(test_pattern);
    for (size_t j = 0; j < rounds; ++j) {
      total_time += test_speed(re, i + 1);
    }
    std::cout << "CPP " << i << " " << (total_time / rounds) << std::endl;
    std::ofstream outfile("results/cpp_std_speed.txt", std::ios::app);
    outfile << (total_time / rounds) << "\n";
    outfile.close();
  }
  return 0;
}
