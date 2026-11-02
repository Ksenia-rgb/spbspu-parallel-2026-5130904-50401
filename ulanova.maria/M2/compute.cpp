#include "compute.hpp"
#include <vector>
#include "interprocess_connect.hpp"
#include "area_calc.hpp"

namespace ulanova
{
  void computeProcess(int readFd, int writeFd, const char*)
  {
    bool running = true;

    while (running)
    {
      request_t req{};
      try
      {
        recvAll(readFd, &req, sizeof(request_t));
      }
      catch (...)
      {
        break;
      }

      if (req.command == cmd_t::EXIT)
      {
        running = false;
        continue;
      }

      if (req.command == cmd_t::CALC)
      {
        response_t resp{};
        try
        {
          std::vector< circle_t > shapesData(req.shapes_count);
          recvAll(readFd, shapesData.data(), shapesData.size() * sizeof(circle_t));
          std::vector< const circle_t* > shapesPtrs(req.shapes_count);
          for (size_t i = 0; i < req.shapes_count; ++i)
          {
            shapesPtrs[i] = &shapesData[i];
          }

          const size_t hits = calc(shapesPtrs.data(), shapesPtrs.data() + req.shapes_count, req.rect, req.tests, req.seed);

          resp.status = status_t::SUCCESS;
          resp.hits = hits;
        }
        catch (...)
        {
          resp.status = status_t::ERROR;
          resp.hits = 0;
        }

        try
        {
          sendAll(writeFd, &resp, sizeof(response_t));
        }
        catch (...)
        {
          break;
        }
      }
    }
  }
}
