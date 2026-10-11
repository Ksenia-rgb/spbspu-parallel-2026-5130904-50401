#include <pthread.h>
#include <random>
#include <cmath>
#include <vector>
#include <iostream>
#include <cstring>

struct circle_t
{
  double r;
  double cx;
  double cy;
};

int main(int argc, char ** argv)
{
  if (argc != 3 && argc != 4) {
    std::cerr << "incorrect number of arguments\n";
    return 1;
  }
  size_t threads = 0;
  size_t tries = 0;
  size_t seed = 0;
  try {
    threads = static_cast< size_t >(std::stoull(argv[1]));
    tries = static_cast< size_t >(std::stoull(argv[2]));
    if (argc == 4) {
      seed = static_cast< size_t >(std::stoull(argv[3]));
    }
  } catch (const std::exception & e) {
    std::cerr << "cannot parse arguments: " << e.what() << "\n";
    return 1;
  }
  if (threads == 0) {
    std::cerr << "inappropriate number of threads\n";
    return 1;
  }
  if (tries == 0) {
    std::cerr << "incorrect number of tries\n";
    return 1;
  }
}
