#include "cli.hpp"
#include <algorithm>
#include <cctype>
#include "area_calc.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <cstring>
#include "compute.hpp"

namespace ulanova
{
  static int parentReadFd = -1;
  static int parentWriteFd = -1;
  static const char* sharedMemoryName = "/ulanova_shm";

  void CLI::skipWs(const std::string& str, std::string::size_type& pos)
  {
    while (pos < str.size() && std::isspace(static_cast< unsigned char >(str[pos])))
    {
      ++pos;
    }
  }

  std::string CLI::readName(const std::string& str, std::string::size_type& pos)
  {
    skipWs(str, pos);
    if (pos >= str.size() || !std::isalpha(static_cast< unsigned char >(str[pos])))
    {
      return "";
    }
    std::string::size_type start = pos;
    while (pos < str.size() && !std::isspace(static_cast< unsigned char >(str[pos])))
    {
      ++pos;
    }
    return str.substr(start, pos - start);
  }

  bool CLI::readInt(const std::string& str, std::string::size_type& pos, int& val)
  {
    skipWs(str, pos);
    if (pos >= str.size())
    {
      return false;
    }
    std::string::size_type next = 0;
    try
    {
      val = std::stoi(str.substr(pos), &next);
      pos += next;
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool CLI::readSize(const std::string& str, std::string::size_type& pos, size_t& val)
  {
    skipWs(str, pos);
    if (pos >= str.size())
    {
      return false;
    }
    std::string::size_type next = 0;
    try
    {
      val = std::stoull(str.substr(pos), &next);
      pos += next;
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool CLI::checkEnd(const std::string& str, std::string::size_type pos)
  {
    skipWs(str, pos);
    return pos >= str.size();
  }

  ptrdiff_t CLI::getCircleIdx(const std::string& name) const
  {
    auto it = std::find(ctx_.names.cbegin(), ctx_.names.cend(), name);
    if (it == ctx_.names.cend())
    {
      return -1;
    }
    return std::distance(ctx_.names.cbegin(), it);
  }

  ptrdiff_t CLI::getSetIdx(const std::string& name) const
  {
    for (size_t i = 0; i < ctx_.sets.size(); ++i)
    {
      if (ctx_.sets[i].name == name)
      {
        return static_cast< ptrdiff_t >(i);
      }
    }
    return -1;
  }

  ptrdiff_t CLI::getProcessIdx(const std::string& name) const
  {
    auto it = std::find(ctx_.activeProcesses.cbegin(), ctx_.activeProcesses.cend(), name);
    if (it == ctx_.activeProcesses.cend())
    {
      return -1;
    }
    return std::distance(ctx_.activeProcesses.cbegin(), it);
  }

  ptrdiff_t CLI::getCalcIdx(const std::string& name) const
  {
    for (size_t i = 0; i < ctx_.calculations.size(); ++i)
    {
      if (ctx_.calculations[i].calcName == name)
      {
        return static_cast< ptrdiff_t >(i);
      }
    }
    return -1;
  }

  void CLI::run()
  {
    std::string line;
    while (std::getline(std::cin, line))
    {
      if (!line.empty())
      {
        processLine(line);
      }
    }
  }

  void CLI::processLine(const std::string& line)
  {
    std::string::size_type pos = 0;
    std::string cmd = readName(line, pos);

    if (cmd.empty())
    {
      return;
    }

    if (cmd == "circle")
    {
      parseCircle(line, pos);
    }
    else if (cmd == "set")
    {
      parseSet(line, pos);
    }
    else if (cmd == "show")
    {
      parseShow(line, pos);
    }
    else if (cmd == "showset")
    {
      parseShowSet(line, pos);
    }
    else if (cmd == "frame")
    {
      parseFrame(line, pos);
    }
    else if (cmd == "frameset")
    {
      parseFrameset(line, pos);
    }
    else if (cmd == "spawn")
    {
      parseSpawn(line, pos);
    }
    else if (cmd == "area-on")
    {
      parseAreaOn(line, pos);
    }
    else if (cmd == "status")
    {
      parseStatus(line, pos);
    }
    else if (cmd == "wait")
    {
      parseWait(line, pos);
    }
    else
    {
      std::cout << "<UNKNOWN COMMAND>\n";
    }
  }

  void CLI::parseCircle(const std::string& line, std::string::size_type& pos)
  {
    std::string name = readName(line, pos);
    int r = 0, x = 0, y = 0;

    if (name.empty() || !readInt(line, pos, r) || !readInt(line, pos, x) || !readInt(line, pos, y)
        || !checkEnd(line, pos) || getCircleIdx(name) != -1 || getSetIdx(name) != -1 || r <= 0)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    ctx_.storage.push_back(circle_t{x, y, r});
    ctx_.names.push_back(name);
  }

  void CLI::parseSet(const std::string& line, std::string::size_type& pos)
  {
    std::string name = readName(line, pos);
    size_t expectedSize = 0;

    if (name.empty() || !readSize(line, pos, expectedSize) || getCircleIdx(name) != -1 || getSetIdx(name) != -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    NamedSet newSet{name, {}};
    for (size_t i = 0; i < expectedSize; ++i)
    {
      std::string shapeName = readName(line, pos);
      ptrdiff_t idx = getCircleIdx(shapeName);
      if (shapeName.empty() || idx == -1)
      {
        std::cout << "<INVALID COMMAND>\n";
        return;
      }
      newSet.circleIndices.push_back(static_cast< size_t >(idx));
    }

    if (!checkEnd(line, pos))
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }
    ctx_.sets.push_back(newSet);
  }

  void CLI::parseShow(const std::string& line, std::string::size_type& pos)
  {
    std::string name = readName(line, pos);
    ptrdiff_t idx = getCircleIdx(name);

    if (name.empty() || !checkEnd(line, pos) || idx == -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    const circle_t& c = ctx_.storage[idx];
    std::cout << c.r << " (" << c.x << " " << c.y << ")\n";
  }

  void CLI::parseShowSet(const std::string& line, std::string::size_type& pos)
  {
    std::string name = readName(line, pos);
    ptrdiff_t idx = getSetIdx(name);

    if (name.empty() || !checkEnd(line, pos) || idx == -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    const NamedSet& s = ctx_.sets[idx];
    std::cout << s.circleIndices.size() << "\n";
    for (size_t cIdx : s.circleIndices)
    {
      const circle_t& c = ctx_.storage[cIdx];
      std::cout << c.r << " (" << c.x << " " << c.y << ")\n";
    }
  }

  void CLI::parseFrame(const std::string& line, std::string::size_type& pos)
  {
    std::string name = readName(line, pos);
    ptrdiff_t idx = getCircleIdx(name);

    if (name.empty() || !checkEnd(line, pos) || idx == -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    const circle_t& c = ctx_.storage[idx];
    const circle_t* arr = {&c};
    rect_t r = bounds(arr, arr + 1);
    std::cout << "(" << r.xmin << " " << r.ymin << ") (" << r.xmax << " " << r.ymax << ")\n";
  }

  void CLI::parseFrameset(const std::string& line, std::string::size_type& pos)
  {
    std::string name = readName(line, pos);
    ptrdiff_t idx = getSetIdx(name);

    if (name.empty() || !checkEnd(line, pos) || idx == -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    const NamedSet& s = ctx_.sets[idx];
    std::vector< const circle_t* > ptrs;
    for (size_t cIdx : s.circleIndices)
    {
      ptrs.push_back(&ctx_.storage[cIdx]);
    }

    rect_t r = bounds(ptrs.data(), ptrs.data() + ptrs.size());
    std::cout << "(" << r.xmin << " " << r.ymin << ") (" << r.xmax << " " << r.ymax << ")\n";
  }

  void CLI::parseSpawn(const std::string& line, std::string::size_type& pos)
  {
    std::string pName = readName(line, pos);
    size_t seedVal = 0;

    if (pName.empty() || !readSize(line, pos, seedVal) || !checkEnd(line, pos) || getProcessIdx(pName) != -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    ctx_.activeProcesses.push_back(pName);
    executeSystemSpawn(pName, static_cast< unsigned int >(seedVal));
  }

  void CLI::parseAreaOn(const std::string& line, std::string::size_type& pos)
  {
    std::string pName = readName(line, pos);
    std::string cName = readName(line, pos);
    std::string setName = readName(line, pos);
    size_t thrs = 0, tries = 0;

    if (pName.empty() || cName.empty() || setName.empty() || !readSize(line, pos, thrs) || !readSize(line, pos, tries)
        || !checkEnd(line, pos) || getProcessIdx(pName) == -1 || getSetIdx(setName) == -1 || getCalcIdx(cName) != -1
        || thrs == 0 || tries == 0)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    CalcActive newCalc{cName, pName, setName, thrs, tries, false, 0.0, 0};
    ctx_.calculations.push_back(newCalc);
    executeSystemAreaOn(ctx_.calculations.back());
  }

  void CLI::parseStatus(const std::string& line, std::string::size_type& pos)
  {
    std::string cName = readName(line, pos);
    ptrdiff_t idx = getCalcIdx(cName);

    if (cName.empty() || !checkEnd(line, pos) || idx == -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    const CalcActive& c = ctx_.calculations[idx];
    if (c.isDone)
    {
      std::cout << c.processName << " " << c.durationMs << " " << c.areaResult << "\n";
    }
    else
    {
      std::cout << c.processName << " <IN PROGRESS>\n";
    }
  }

  void CLI::parseWait(const std::string& line, std::string::size_type& pos)
  {
    std::string cName = readName(line, pos);
    ptrdiff_t idx = getCalcIdx(cName);

    if (cName.empty() || !checkEnd(line, pos) || idx == -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    CalcActive& c = ctx_.calculations[idx];
    if (!c.isDone)
    {
      executeSystemWait(c);
    }
    std::cout << c.processName << " " << c.durationMs << " " << c.areaResult << "\n";
  }

  void CLI::executeSystemSpawn(const std::string&, unsigned int)
  {
    int pipeToChild[2];
    int pipeToParent[2];

    if (pipe(pipeToChild) == -1 || pipe(pipeToParent) == -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      return;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
      std::cout << "<INVALID COMMAND>\n";
      ::close(pipeToChild[0]);
      ::close(pipeToChild[1]);
      ::close(pipeToParent[0]);
      ::close(pipeToParent[1]);
      return;
    }

    if (pid == 0)
    {
      ::close(pipeToChild[1]);
      ::close(pipeToParent[0]);

      computeProcess(pipeToChild[0], pipeToParent[1], sharedMemoryName);

      ::close(pipeToChild[0]);
      ::close(pipeToParent[1]);
      std::exit(0);
      else
      {
        ::close(pipeToChild[0]);
        ::close(pipeToParent[1]);

        parentWriteFd = pipeToChild[1];
        parentReadFd = pipeToParent[0];
      }
    }

    void CLI::executeSystemAreaOn(CalcActive & calc)
    {
      if (parentWriteFd == -1 || parentReadFd == -1)
      {
        std::cout << "<INVALID COMMAND>\n";
        return;
      }

      ptrdiff_t setIdx = getSetIdx(calc.setName);
      if (setIdx == -1)
      {
        return;
      }

      const NamedSet& s = ctx_.sets[setIdx];
      size_t shapesCount = s.circleIndices.size();

      request_t req{};
      req.command = cmd_t::CALC;
      req.shapesCount = shapesCount;
      req.tests = calc.tries;
      req.seed = 42;

      std::vector< const circle_t* > ptrs;
      for (size_t cIdx : s.circleIndices)
      {
        ptrs.push_back(&ctx_.storage[cIdx]);
      }
      req.rect = bounds(ptrs.data(), ptrs.data() + ptrs.size());

      size_t shmSize = shapesCount * sizeof(circle_t);
      Mapping shm;
      shm.create(sharedMemoryName, shmSize);

      circle_t* shmBuffer = static_cast< circle_t* >(shm.getPtr());
      for (size_t i = 0; i < shapesCount; ++i)
      {
        shmBuffer[i] = ctx_.storage[s.circleIndices[i]];
      }

      try
      {
        sendAll(parentWriteFd, &req, sizeof(request_t));
      }
      catch (...)
      {
        std::cout << "<INVALID COMMAND>\n";
      }
    }

    void CLI::executeSystemWait(CalcActive & calc)
    {
      if (parentReadFd == -1)
      {
        return;
      }

      response_t resp{};
      auto startTime = std::chrono::steady_clock::now();

      try
      {
        recvAll(parentReadFd, &resp, sizeof(response_t));
        auto endTime = std::chrono::steady_clock::now();

        if (resp.status == status_t::SUCCESS)
        {
          calc.isDone = true;
          calc.durationMs = std::chrono::duration_cast< std::chrono::milliseconds >(endTime - startTime).count();
          ptrdiff_t setIdx = getSetIdx(calc.setName);
          std::vector< const circle_t* > ptrs;
          for (size_t cIdx : ctx_.sets[setIdx].circleIndices)
          {
            ptrs.push_back(&ctx_.storage[cIdx]);
          }
          rect_t r = bounds(ptrs.data(), ptrs.data() + ptrs.size());
          double boxArea = (r.xmax - r.xmin) * (r.ymax - r.ymin);

          calc.areaResult = (static_cast< double >(resp.hits) / calc.tries) * boxArea;
        }
      }
      catch (...)
      {
        std::cout << "<INVALID COMMAND>\n";
      }
    }
  }
