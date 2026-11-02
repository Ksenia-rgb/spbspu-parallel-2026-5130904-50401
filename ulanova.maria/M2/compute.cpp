#include "compute.hpp"
#include <vector>
#include "interprocess_connect.hpp"
#include "area_calc.hpp"

namespace ulanova
{
  void computeProcess(int readFd, int writeFd, const char* shmName)
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
        runnig = false;
        continue;
      }

      if (req.command == cmd_t::CALC)
      {
        response_t resp{};
        try
        {
          size_t shmSize = req.shapesCount * sizeof(circle_t);

          Mapping shm;
          shm.open(shmName, shmSize);

          const circle_t* shapesData = static_cast< const circle_t* >(shm.getPtr());
          std::vector< const circle_t* > shapesPtrs(req.shapesCount);
          for (size_t i = 0; i < req.shapesCount; ++i)
          {
            shapesPtrs[i] = &shapesData[i];
          }

          size_t hits = calc(shapesPtrs.data(), shapesPtrs.data() + req.shapesCount, req.rect, req.tests, req.seed);

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

