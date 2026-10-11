#include <iostream>
#include <string>

int zaglushka(int a, int x, int y) {
  return x + y + a;
}

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
  int a, b, x, y;
  while (!std::cin.eof()) {
    std::cin >> a >> b >> x >> y;
    zaglushka(a, x, y);
  }
}
