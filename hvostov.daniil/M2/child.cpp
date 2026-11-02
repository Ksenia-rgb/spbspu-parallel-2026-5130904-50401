#include "child.hpp"
#include "calculation.hpp"
#include <unistd.h>
#include <vector>
#include <cstring>

int hvostov::runChild(size_t seed, int rd, int wr, ShmLayout* layout)
{
  while (true) {
    int err = 0;
    char sig = 0;
    const size_t n = recv(err, rd, &sig, 1);
    if (n == 0)
      break;
    if (n != 1)
      return 1;

    const size_t idx = layout->read_idx % MAX_SLOTS;
    ShmSlot& slot = layout->slots[idx];

    if (slot.req.shape_count == 0 || slot.req.shape_count > MAX_SHAPES) {
      return 1;
    }

    std::vector< shape_t > shapes(slot.shapes, slot.shapes + slot.req.shape_count);
    const size_t threads = slot.req.threads > 0 ? static_cast< size_t >(slot.req.threads) : 1;
    const size_t calc_seed = slot.req.seed != 0 ? slot.req.seed : seed;

    double time_ms = 0.0;
    const double area = calcArea(shapes, threads, slot.req.tries, calc_seed, time_ms);

    slot.res.result = area;
    slot.res.time_ms = time_ms;
    std::memcpy(slot.res.calc_name, slot.req.calc_name, sizeof(slot.res.calc_name));

    layout->read_idx++;

    char done = 1;
    if (send(err, wr, &done, 1) != 1)
      return 1;
  }
  close(rd);
  close(wr);
  return 0;
}
