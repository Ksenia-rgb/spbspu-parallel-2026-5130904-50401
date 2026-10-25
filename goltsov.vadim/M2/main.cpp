#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string>
#include <map>
#include <vector>
#include "shared_circles.hpp"

const char* const SHMN = "shm-temp";

namespace goltsov
{
  std::map< std::string, size_t > name_idx_map;
  std::map< std::string, std::vector< std::string > > sets;

  struct CirclePtr
  {
    std::string name;
    size_t idx;
  };

  void circle(goltsov::SharedCircles& shared_circles)
  {
    std::string name;
    size_t r;
    long long x, y;
    std::cin >> name >> r >> x >> y;
    shared_circles.new_circle(r, x, y);
    name_idx_map.insert({name, shared_circles.get_count_circles()});
  }

  void set(goltsov::SharedCircles& shared_circles)
  {
    std::string name;
    size_t set_size;
    std::vector< std::string > shapes;
    std::cin >> name >> set_size;
    for (size_t i = 0; i < set_size; ++i)
    {
      std::string shapei;
      std::cin >> shapei;
      shapes.push_back(shapei);
    }
    sets[name] = shapes;
  }

  void show(goltsov::SharedCircles& shared_circles)
  {
    std::string shape_name;
    std::cin >> shape_name;
    Circle circle = shared_circles.get_data()[name_idx_map.at(shape_name)];
    std::cout << circle << "\n";
  }

  void showset(goltsov::SharedCircles& shared_circles)
  {
    std::string set_name;
    std::cin >> set_name;
    std::vector< std::string > set = sets.at(set_name);
    for (size_t i = 0; i < set.size(); ++i)
    {
      std::cout << name_idx_map.at(set[i]) << "\n";
    }
  }

}

int main()
{
  goltsov::SharedCircles shared_circles = goltsov::SharedCircles(SHMN);

}
