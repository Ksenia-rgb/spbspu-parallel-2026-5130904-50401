#include <iostream>
#include <cstddef>

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

