#include "worker.hpp"

#include <chrono>
#include <cstddef>
#include <vector>

#include "../common/shapes.hpp"

#include "ipc.hpp"

void burukov::runWorkerLoop(int read_fd, int write_fd, long long seed)
{
  constexpr double MS_PER_SEC = 1000.0;
  task_header_t header = {0, 0, 0, 0, 0};

  while (readAll(read_fd, &header, sizeof(header)))
  {
    if (header.type == 0)
    {
      break;
    }

    std::vector< shape_data_t > shapes(header.shape_count);
    if (header.shape_count > 0)
    {
      const std::size_t bytes = static_cast< std::size_t >(header.shape_count) * sizeof(shape_data_t);
      if (!readAll(read_fd, shapes.data(), bytes))
      {
        break;
      }
    }

    const auto start_time = std::chrono::steady_clock::now();
    const double area_val = calculateUnionArea(header.threads, header.tries, seed, shapes);
    const auto end_time = std::chrono::steady_clock::now();

    const std::chrono::duration< double > elapsed = end_time - start_time;
    const double duration_ms = elapsed.count() * MS_PER_SEC;

    const task_result_t result = {header.calc_id, duration_ms, area_val};
    if (!writeAll(write_fd, &result, sizeof(result)))
    {
      break;
    }
  }
}
