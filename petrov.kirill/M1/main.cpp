#include <iostream>
#include <string>

int main(int argc, char** argv) {
  int threads, tires, seed;
  if (argc == 4) {
    threads = std::stoi(argv[1]);
    tires = std::stoi(argv[2]);
    seed = std::stoi(argv[3]);
  } else {
    threads = std::stoi(argv[1]);
    tires = std::stoi(argv[2]);
    seed = 0;
  }

}
