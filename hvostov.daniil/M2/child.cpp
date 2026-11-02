#include "child.hpp"
#include <unistd.h>
#include <vector>
#include <cstring>
#include "calculation.hpp"

int hvostov::runChild(size_t seed, int rd, int wr, ShmLayout* layout)
{
  while (true) {
    int err = 0;
    char sig = 0;
    const size_t n = recv(err, rd, &sig, 1);
    if (n == 0) {
      break;
    }
    if (n != 1) {
      return 1;
    }

    if (layout->req.shape_count == 0 || layout->req.shape_count > MAX_SHAPES) {
      return 1;
    }

    std::vector< shape_t > shapes(layout->shapes, layout->shapes + layout->req.shape_count);

    const size_t threads = static_cast< size_t >(layout->req.threads > 0 ? layout->req.threads : 1);
    const size_t tries = layout->req.tries;
    const size_t calc_seed = (layout->req.seed != 0) ? layout->req.seed : seed;

    double time_ms = 0.0;
    const double area = calcArea(shapes, threads, tries, calc_seed, time_ms);

    layout->res.result = area;
    layout->res.time_ms = time_ms;
    std::memcpy(layout->res.calc_name, layout->req.calc_name, sizeof(layout->res.calc_name));

    char done = 1;
    if (send(err, wr, &done, 1) != 1) {
      return 1;
    }
  }

  close(rd);
  close(wr);
  return 0;
}
