#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <map>
#include <string>
#include <vector>
#include "geometry.hpp"

namespace studilova
{
  using circles_t = std::map< std::string, circle_t >;
  using circle_set_t = std::vector< std::string >;
  using sets_t = std::map< std::string, circle_set_t >;
}

#endif
