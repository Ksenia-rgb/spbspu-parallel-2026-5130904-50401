#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "shared_circles.hpp"

namespace goltsov
{
  void circle(SharedCircles& shared_circles);
  void set(SharedCircles& shared_circles);
  void show(SharedCircles& shared_circles);
  void showset(SharedCircles& shared_circles);
  void frame(SharedCircles& shared_circles);
  void frameset(SharedCircles& shared_circles);
  void spawn(SharedCircles& shared_circles);
  void areas_on(SharedCircles& shared_circles);
  void status(SharedCircles& shared_circles);
  void wait(SharedCircles& shared_circles);
  void shutdown();
}

#endif
