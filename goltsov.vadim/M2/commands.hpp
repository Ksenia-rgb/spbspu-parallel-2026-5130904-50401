#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "shared_circles.hpp"

namespace goltsov
{
  void circle(SharedCircles& shared_circles);
  void set(SharedCircles&);
  void show(SharedCircles& shared_circles);
  void showset(SharedCircles& shared_circles);
  void frame(SharedCircles& shared_circles);
  void frameset(SharedCircles& shared_circles);
  void spawn(SharedCircles&);
  void areasOn(SharedCircles& shared_circles);
  void status(SharedCircles&);
  void wait(SharedCircles&);
  void shutdown();
}

#endif
