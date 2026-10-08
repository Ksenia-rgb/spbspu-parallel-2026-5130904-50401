#include <iostream>
#include <cstddef>
#include <vector>

namespace petrov
{
  struct p_t {
    int x_;
    int y_;
  };

  struct Elips {
    p_t center_;
    int radius1_;
    int radius2_;

    Elips(int r1, int r2, int x, int y)
    {
      radius1_ = r1;
      radius2_ = r2 > 0 ? r2 : r1 ;
      center_ = p_t{x, y};
    }
  };
}

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

  std::vector< petrov::Elips > shapes;

  int r1, r2, x, y;
  while (std::cin >> r1 >> r2 >> x >> y)
  {
    if (r1 <= 0 || r2 < 0)
    {
      std::cerr << "Radius invalid\n";
      return 1;
    }

    shapes.push_back(petrov::Elips(r1, r2, x, y));
  }
  
  if (!std::cin.eof())
  {
    std::cerr << "Failed to parse figure\n";
    return 1;
  }

  if (shapes.empty())
  {
    std::cerr << "No figures provided\n";
    return 1;
  }
  
  return 0;
}

