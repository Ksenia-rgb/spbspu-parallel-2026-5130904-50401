#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string>
#include <map>
#include <vector>
#include <limits>
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
    Circle* circle = shared_circles.get_data() + name_idx_map.at(shape_name);
    std::cout << *circle << "\n";
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

  void frame(goltsov::SharedCircles& shared_circles)
  {
    std::string shape_name;
    std::cin >> shape_name;
    Circle* circle = shared_circles.get_data() + name_idx_map.at(shape_name);
    std::cout << "(" << circle->x - circle->r << " " << circle->y - circle->r << ") (" << circle->x + circle->r << " " << circle->y + circle->r << ")" << "\n";
  }

  void frameset(goltsov::SharedCircles& shared_circles)
  {
    std::string set_name;
    std::cin >> set_name;
    std::vector< std::string > set = sets.at(set_name);
    long long min_x = std::numeric_limits< long long >::max();
    long long max_x = std::numeric_limits< long long >::min();
    long long min_y = std::numeric_limits< long long >::max();
    long long max_y = std::numeric_limits< long long >::min();
    for (size_t i = 0; i < set.size(); ++i)
    {
      Circle* circle = shared_circles.get_data() + name_idx_map.at(set[i]);
      min_x = std::min< long long >(min_x, circle->x - circle->r);
      max_x = std::max< long long >(max_x, circle->x + circle->r);
      min_y = std::min< long long >(min_y, circle->y - circle->r);
      max_y = std::max< long long >(max_y, circle->y + circle->r);
    }
    std::cout << "(" << min_x << " " << min_y << ") (" << max_x << " " << max_y << ")" << "\n";
  }
}

int main()
{
  goltsov::SharedCircles shared_circles = goltsov::SharedCircles(SHMN);

}
