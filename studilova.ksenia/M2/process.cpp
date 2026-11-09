#include "process.hpp"

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <exception>
#include <vector>

#include "area.hpp"
#include "geometry.hpp"
#include "pipe.hpp"

namespace studilova
{
  namespace detail
  {
    class Clicker
    {
      public:
        Clicker();
        double millisec() const;

      private:
        std::chrono::time_point< std::chrono::steady_clock > start_;
    };
  }
}

studilova::detail::Clicker::Clicker():
  start_(std::chrono::steady_clock::now())
{}

double studilova::detail::Clicker::millisec() const
{
  const auto finish = std::chrono::steady_clock::now();

  return std::chrono::duration< double, std::milli >(finish - start_).count();
}

void studilova::runProcess(int readDescriptor, int writeDescriptor,
    unsigned int seed)
{
  while (true)
  {
    request_t request{};
    int error = 0;

    const auto received = readAll(error, readDescriptor, &request, sizeof(request));

    if (error != 0)
    {
      throw std::runtime_error(std::strerror(error));
    }

    if (received == 0)
    {
      return;
    }

    if (received != sizeof(request))
    {
      throw std::runtime_error("Incomplete request");
    }

    std::vector< circle_t > circles(request.circleCount);

    for (circle_t& circle: circles)
    {
      const auto circleBytes = readAll(error, readDescriptor, &circle, sizeof(circle));

      if (error != 0)
      {
        throw std::runtime_error(std::strerror(error));
      }

      if (circleBytes != sizeof(circle))
      {
        throw std::runtime_error("Incomplete circle");
      }
    }

    response_t response{};
    response.calculationId = request.calculationId;

    try
    {
      const detail::Clicker timer;

      response.area = calculateArea(circles, request.threads, request.tries, seed);

      response.milliseconds = timer.millisec();
      response.success = true;
    }
    catch (const std::exception&)
    {
      response.success = false;
      response.area = 0.0;
      response.milliseconds = 0.0;
    }

    const auto sent = writeAll(error, writeDescriptor, &response, sizeof(response));

    if (error != 0)
    {
      throw std::runtime_error(std::strerror(error));
    }

    if (sent != sizeof(response))
    {
      throw std::runtime_error("Incomplete response");
    }
  }
}
