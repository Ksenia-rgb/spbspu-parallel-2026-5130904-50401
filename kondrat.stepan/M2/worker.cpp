#include "worker.hpp"
#include <cerrno>
#include <chrono>
#include <exception>
#include <random>
#include <ratio>
#include <stdexcept>
#include <system_error>
#include <vector>
#include <signal.h>
#include <unistd.h>
#include "calculation.hpp"
#include "channel.hpp"
#include "protocol.hpp"
#include "shape.hpp"

int kondrat::workerLoop(int request_descriptor, int result_descriptor, long long seed)
{
  int exit_code = 0;

  try
  {
    struct sigaction action = {};
    action.sa_handler = SIG_IGN;

    if (sigemptyset(&action.sa_mask) == -1 || sigaction(SIGPIPE, &action, nullptr) == -1)
    {
      throw std::system_error(errno, std::generic_category(), "Cannot configure SIGPIPE");
    }

    std::default_random_engine generator(static_cast< std::default_random_engine::result_type >(seed));
    Request request = {};

    while (readFull(request_descriptor, &request, sizeof(request)))
    {
      if (request.kind == RequestKind::Stop)
      {
        break;
      }

      if (request.kind != RequestKind::Area || request.shape_count == 0)
      {
        throw std::invalid_argument("Invalid calculation request");
      }

      std::vector< Shape > shapes(request.shape_count);

      if (!readFull(request_descriptor, shapes.data(), shapes.size() * sizeof(Shape)))
      {
        throw std::runtime_error("Missing request shapes");
      }

      const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
      const double area = calculateArea(shapes, request.threads, request.tries, generator);
      const std::chrono::steady_clock::time_point finish = std::chrono::steady_clock::now();
      const double time_ms = std::chrono::duration< double, std::milli >(finish - start).count();
      const Response response = {request.calculation_id, time_ms, area};

      writeFull(result_descriptor, &response, sizeof(response));
    }
  }
  catch (const std::exception &)
  {
    exit_code = 1;
  }

  close(request_descriptor);
  close(result_descriptor);

  return exit_code;
}
