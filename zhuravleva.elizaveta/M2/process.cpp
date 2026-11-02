
#include "process.hpp"
#include "ipc.hpp"
#include "monte_carlo.hpp"
#include <cerrno>
#include <csignal>
#include <limits>
#include <unistd.h>
#include <sys/wait.h>

namespace
{
  void closeIfOpen(int fd)
  {
    if (fd >= 0)
    {
      close(fd);
    }
  }

  void waitForChild(pid_t pid)
  {
    int status = 0;
    while (waitpid(pid, &status, 0) == -1)
    {
      if (errno != EINTR)
      {
        break;
      }
    }
  }

  void markFailed(zhuravleva::ProcessStorage& storage, const std::string& name)
  {
    for (auto& item : storage.calculations)
    {
      if (item.second.process_name == name && item.second.state == zhuravleva::CalcState::pending)
      {
        item.second.state = zhuravleva::CalcState::failed;
      }
    }
  }

  void childLoop(int read_fd, int write_fd, unsigned int seed)
  {
    constexpr std::uint64_t max_shapes = 1000000;
    while (true)
    {
      zhuravleva::AreaRequest request{};
      if (!zhuravleva::readAll(read_fd, &request, sizeof(request)))
      {
        break;
      }
      if (request.shape_count > max_shapes || request.threads == 0 || request.tries == 0
          || request.shape_count > std::numeric_limits< std::size_t >::max())
      {
        break;
      }
      std::vector< zhuravleva::Shape > shapes(static_cast< std::size_t >(request.shape_count));
      if (!shapes.empty() && !zhuravleva::readAll(read_fd, shapes.data(), shapes.size() * sizeof(shapes.front())))
      {
        break;
      }
      zhuravleva::AreaResult result{};
      result.id = request.id;
      try
      {
        result.area = zhuravleva::calculateArea(shapes, static_cast< std::size_t >(request.threads),
            static_cast< std::size_t >(request.tries), static_cast< std::size_t >(seed) + request.id);
        result.success = 1;
      }
      catch (...)
      {
        result.success = 0;
      }
      if (!zhuravleva::writeAll(write_fd, &result, sizeof(result)))
      {
        break;
      }
    }
  }

  bool collectResult(zhuravleva::ProcessStorage& storage, const std::string& process_name)
  {
    const auto process_it = storage.processes.find(process_name);
    if (process_it == storage.processes.end())
    {
      return false;
    }
    zhuravleva::AreaResult result{};
    if (!zhuravleva::readAll(process_it->second.read_fd, &result, sizeof(result)))
    {
      markFailed(storage, process_name);
      return false;
    }
    for (auto& item : storage.calculations)
    {
      if (item.second.id == result.id && item.second.process_name == process_name)
      {
        item.second.area = result.area;
        item.second.state = result.success != 0 ? zhuravleva::CalcState::done : zhuravleva::CalcState::failed;
        return true;
      }
    }
    markFailed(storage, process_name);
    return false;
  }
}

bool zhuravleva::spawnProcess(ProcessStorage& storage, const std::string& name, unsigned int seed)
{
  if (name.empty() || storage.processes.find(name) != storage.processes.end())
  {
    return false;
  }
  int to_child[2] = {-1, -1};
  int from_child[2] = {-1, -1};
  if (pipe(to_child) == -1)
  {
    return false;
  }
  if (pipe(from_child) == -1)
  {
    closeIfOpen(to_child[0]);
    closeIfOpen(to_child[1]);
    return false;
  }
  const pid_t pid = fork();
  if (pid == -1)
  {
    closeIfOpen(to_child[0]);
    closeIfOpen(to_child[1]);
    closeIfOpen(from_child[0]);
    closeIfOpen(from_child[1]);
    return false;
  }
  if (pid == 0)
  {
    closeIfOpen(to_child[1]);
    closeIfOpen(from_child[0]);
    for (const auto& item : storage.processes)
    {
      closeIfOpen(item.second.write_fd);
      closeIfOpen(item.second.read_fd);
    }
    childLoop(to_child[0], from_child[1], seed);
    closeIfOpen(to_child[0]);
    closeIfOpen(from_child[1]);
    _exit(0);
  }
  closeIfOpen(to_child[0]);
  closeIfOpen(from_child[1]);
  storage.processes.emplace(name, Process{pid, to_child[1], from_child[0], seed});
  return true;
}

bool zhuravleva::killProcess(ProcessStorage& storage, const std::string& name)
{
  const auto it = storage.processes.find(name);
  if (it == storage.processes.end())
  {
    return false;
  }
  const Process process = it->second;
  if (kill(process.pid, SIGTERM) == -1 && errno != ESRCH)
  {
    return false;
  }
  closeIfOpen(process.write_fd);
  closeIfOpen(process.read_fd);
  waitForChild(process.pid);
  markFailed(storage, name);
  storage.processes.erase(it);
  return true;
}

void zhuravleva::shutdownProcesses(ProcessStorage& storage)
{
  for (auto& item : storage.processes)
  {
    kill(item.second.pid, SIGTERM);
    closeIfOpen(item.second.write_fd);
    closeIfOpen(item.second.read_fd);
  }
  for (auto& item : storage.processes)
  {
    waitForChild(item.second.pid);
    markFailed(storage, item.first);
  }
  storage.processes.clear();
}

bool zhuravleva::submitArea(ProcessStorage& storage, const std::string& process_name, const std::string& calc_name,
    const std::vector< Shape >& shapes, std::size_t threads, std::size_t tries)
{
  const auto it = storage.processes.find(process_name);
  if (it == storage.processes.end() || calc_name.empty()
      || storage.calculations.find(calc_name) != storage.calculations.end() || shapes.empty() || threads == 0
      || tries == 0)
  {
    return false;
  }
  AreaRequest request{};
  request.id = storage.next_id;
  request.shape_count = shapes.size();
  request.threads = threads;
  request.tries = tries;
  if (!writeAll(it->second.write_fd, &request, sizeof(request))
      || !writeAll(it->second.write_fd, shapes.data(), shapes.size() * sizeof(Shape)))
  {
    killProcess(storage, process_name);
    return false;
  }
  storage.calculations.emplace(calc_name, Calculation{request.id, process_name, CalcState::pending, 0.0});
  ++storage.next_id;
  return true;
}

bool zhuravleva::waitCalculation(ProcessStorage& storage, const std::string& calc_name, double& area)
{
  const auto it = storage.calculations.find(calc_name);
  if (it == storage.calculations.end())
  {
    return false;
  }
  const std::string process_name = it->second.process_name;
  while (it->second.state == CalcState::pending)
  {
    if (!collectResult(storage, process_name))
    {
      return false;
    }
  }
  if (it->second.state != CalcState::done)
  {
    return false;
  }
  area = it->second.area;
  return true;
}
