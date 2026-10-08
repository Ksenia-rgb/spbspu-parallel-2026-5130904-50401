#include <iostream>
#include <cstddef>

struct p_t {
  int x_;
  int y_;
};

struct Circle {
  p_t center_;
  int radius_;
};

struct Elips : Circle {
  int radius2_;

  Elips(int r1, int r2, int x, int y)
  {
    radius_ = r1;
    radius2_ = r2;
    center_ = p_t{x, y};
  }
};

int main(int argc, char const *argv[])
{
  if (argc < 3)
  {
    std::cerr << "not enough arguments\n";
    return 1;
  }

  size_t threads, tries, seed;
  try
  {
    threads = std::stoll(argv[1]);
    tries = std::stoll(argv[2]);

    if (argc == 4)
    {
      seed = std::stoll(argv[3]);
    }
    else
    {
      seed = 0;
    }
  }
  catch(...)
  {
    std::cerr << "Invalid arguments" << '\n';
  }
  
  return 0;
}

