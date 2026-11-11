#include "process.hpp"

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <exception>
#include <vector>
#include <cerrno>
#include <exception>
#include <utility>
#include <unistd.h>
#include <ostream>
#include <sys/types.h>
#include <sys/wait.h>

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

void studilova::runProcess(int read_descriptor, int write_descriptor, unsigned int seed)
{
  while (true)
  {
    request_t request{};
    int error = 0;

    const auto received = readAll(error, read_descriptor, &request, sizeof(request));

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

    std::vector< circle_t > circles(request.circle_count);

    for (circle_t& circle : circles)
    {
      const auto circleBytes = readAll(error, read_descriptor, &circle, sizeof(circle));

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
    response.calculation_id = request.calculation_id;

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

    const auto sent = writeAll(error, write_descriptor, &response, sizeof(response));

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

studilova::process_t studilova::startProcess(unsigned int seed, const processes_t& processes)
{
  int request_pipe[2] = {-1, -1};

  if (pipe(request_pipe) == -1)
  {
    throw std::runtime_error(std::strerror(errno));
  }

  Descriptor request_read(request_pipe[0]);
  Descriptor request_write(request_pipe[1]);

  int response_pipe[2] = {-1, -1};

  if (pipe(response_pipe) == -1)
  {
    throw std::runtime_error(std::strerror(errno));
  }

  Descriptor response_read(response_pipe[0]);
  Descriptor response_write(response_pipe[1]);

  const pid_t pid = fork();

  if (pid == -1)
  {
    throw std::runtime_error(std::strerror(errno));
  }

  if (pid == 0)
  {
    request_write = Descriptor();
    response_read = Descriptor();

    for (const auto& entry : processes)
    {
      const int write_descriptor = entry.second.write_descriptor.get();
      const int read_descriptor = entry.second.read_descriptor.get();

      if (write_descriptor != -1)
      {
        close(write_descriptor);
      }

      if (read_descriptor != -1)
      {
        close(read_descriptor);
      }
    }

    int exit_code = 0;

    try
    {
      runProcess(request_read.get(), response_write.get(), seed);
    }
    catch (const std::exception&)
    {
      exit_code = 1;
    }

    _exit(exit_code);
  }

  request_read = Descriptor();
  response_write = Descriptor();

  return process_t{pid, std::move(request_write), std::move(response_read)};
}

void studilova::receiveResult(process_t& process, const std::string& process_name, calculations_t& calculations)
{
  try
  {
    if (process.read_descriptor.get() == -1)
    {
      throw std::runtime_error("Process is unavailable");
    }

    response_t response{};
    int error = 0;

    const auto received = readAll(error, process.read_descriptor.get(), &response, sizeof(response));

    if (error != 0)
    {
      throw std::runtime_error(std::strerror(error));
    }

    if (received != sizeof(response))
    {
      throw std::runtime_error("Cannot receive calculation result");
    }

    for (auto& entry : calculations)
    {
      calculation_t& calculation = entry.second;

      if (calculation.id == response.calculation_id && calculation.process_name == process_name)
      {
        calculation.completed = true;
        calculation.success = response.success;
        calculation.area = response.area;
        calculation.milliseconds = response.milliseconds;
        return;
      }
    }

    throw std::runtime_error("Unknown calculation result");
  }
  catch (...)
  {
    process.write_descriptor = Descriptor();
    process.read_descriptor = Descriptor();

    for (auto& entry : calculations)
    {
      calculation_t& calculation = entry.second;

      if (calculation.process_name == process_name && !calculation.completed)
      {
        calculation.completed = true;
        calculation.success = false;
      }
    }
    throw;
  }
}

int studilova::finishProcesses(processes_t& processes, calculations_t& calculations, std::ostream& errors)
{
  for (auto& entry : processes)
  {
    entry.second.write_descriptor = Descriptor();
  }

  int exit_code = 0;

  for (auto& entry : processes)
  {
    const std::string& process_name = entry.first;
    process_t& process = entry.second;

    try
    {
      for (auto& calculation_entry : calculations)
      {
        const calculation_t& calculation = calculation_entry.second;

        if (calculation.process_name != process_name)
        {
          continue;
        }

        while (!calculation.completed)
        {
          receiveResult(process, process_name, calculations);
        }

        if (!calculation.success)
        {
          exit_code = 1;
        }
      }
    }
    catch (const std::exception& e)
    {
      errors << e.what() << '\n';
      exit_code = 1;
    }

    process.read_descriptor = Descriptor();

    int status = 0;
    pid_t waited = -1;

    do
    {
      waited = waitpid(process.pid, &status, 0);
    }
    while (waited == -1 && errno == EINTR);

    if (waited == -1)
    {
      errors << std::strerror(errno) << '\n';
      exit_code = 1;
    }
    else if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
      errors << "Child process failed\n";
      exit_code = 1;
    }
  }
  return exit_code;
}
