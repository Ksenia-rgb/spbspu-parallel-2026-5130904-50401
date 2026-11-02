#include "process.hpp"

#include <cerrno>
#include <csignal>
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

  void childLoop(int read_fd)
  {
    constexpr std::size_t buffer_size = 256;
    char buffer[buffer_size];
    while (true)
    {
      const ssize_t count = read(read_fd, buffer, sizeof(buffer));
      if (count > 0)
      {
        continue;
      }
      if (count == -1 && errno == EINTR)
      {
        continue;
      }
      break;
    }
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
    childLoop(to_child[0]);
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
  storage.processes.erase(it);
  return true;
}

void zhuravleva::shutdownProcesses(ProcessStorage& storage)
{
  for (auto& item : storage.processes)
  {
    closeIfOpen(item.second.write_fd);
    item.second.write_fd = -1;
  }
  for (auto& item : storage.processes)
  {
    waitForChild(item.second.pid);
    closeIfOpen(item.second.read_fd);
  }
  storage.processes.clear();
}
