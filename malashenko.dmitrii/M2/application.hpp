#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "geometry.hpp"

#include <map>
#include <string>
#include <vector>

namespace malashenko
{
  struct ApplicationState
  {
    std::map< std::string, Circle > circles;

    std::map< std::string, std::vector< std::string > > sets;
  };

  void createCircle(ApplicationState& state, const std::string& name, int radius, int x, int y);

  void createSet(ApplicationState& state, const std::string& name, const std::vector< std::string >& circle_names);

  const Circle& getCircle(const ApplicationState& state, const std::string& name);

  const std::vector< std::string >& getSet(const ApplicationState& state, const std::string& name);

  std::vector< Circle > collectSetCircles(const ApplicationState& state, const std::string& name);

  BoundingBox getCircleFrame(const ApplicationState& state, const std::string& name);

  BoundingBox getSetFrame(const ApplicationState& state, const std::string& name);
}


#endif