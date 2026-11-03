#include <cstddef>
#include <future>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "utils.hpp"

int main(int argc, char *argv[])
{
  if (argc < 3 || argc > 4) {
    std::cerr << "too few args\n";
    return 1;
  }

  long long threadsInput = 0;
  long long triesInput = 0;
  long long seedInput = 0;

  try {
    size_t pos = 0;
    threadsInput = std::stoll(argv[1], &pos);
    if (argv[1][pos] != '\0') {
      std::cerr << "bad args\n";
      return 1;
    }

    pos = 0;
    triesInput = std::stoll(argv[2], &pos);
    if (argv[2][pos] != '\0') {
      std::cerr << "bad args\n";
      return 1;
    }

    if (argc == 4) {
      pos = 0;
      seedInput = std::stoll(argv[3], &pos);
      if (argv[3][pos] != '\0') {
        std::cerr << "bad args\n";
        return 1;
      }
    }
  } catch (...) {
    std::cerr << "bad params\n";
    return 1;
  }

  if (triesInput <= 0 || seedInput < 0 || threadsInput < 0) {
    std::cerr << "bad params\n";
    return 1;
  }

  size_t tries = static_cast< size_t >(triesInput);
  size_t seed = static_cast< size_t >(seedInput);
  size_t requestedThreads = static_cast< size_t >(threadsInput);

  size_t threads = (requestedThreads == 0) ? 1 : requestedThreads;

  threads = std::min(threads, tries);

  size_t hw = std::thread::hardware_concurrency();
  size_t maxThreads = (hw == 0) ? 16 : hw;
  threads = std::min(threads, maxThreads);
  levkin::CirclesWithRanges cv;
  try {
    cv = levkin::getData(std::cin);
  } catch (const std::exception &e) {
    std::cerr << "bad input: " << e.what() << '\n';
    return 2;
  }

  if (cv.circles.empty()) {
    std::cout << "0 0\n";
    return 0;
  }

  size_t baseTries = tries / threads;
  size_t remainder = tries % threads;

  std::vector< std::future< levkin::AnyAllResult > > futures;
  futures.reserve(threads);

  for (size_t i = 0; i < threads; ++i) {
    size_t threadTries = baseTries + (i < remainder ? 1 : 0);
    if (threadTries > 0) {
      futures.emplace_back(std::async(std::launch::async, levkin::runTests, threadTries, seed + i, std::cref(cv)));
    }
  }

  levkin::AnyAllResult hit{};
  for (auto &ft : futures) {
    hit += ft.get();
  }

  double boxArea = static_cast< double >(cv.rangeArea());
  double resAny = (static_cast< double >(hit.any) / static_cast< double >(tries)) * boxArea;
  double resAll = (static_cast< double >(hit.all) / static_cast< double >(tries)) * boxArea;

  std::cout.precision(std::numeric_limits< double >::max_digits10);
  std::cout << resAny << " " << resAll << "\n";

  return 0;
}
