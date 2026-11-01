#include "command_reader.hpp"
#include <cerrno>
#include <cstddef>
#include <stdexcept>
#include <system_error>
#include <vector>
#include <poll.h>
#include <unistd.h>

kondrat::CommandReader::CommandReader():
  pending_(),
  finished_(false)
{}

bool kondrat::CommandReader::readLine(std::string & line, Jobs & jobs)
{
  constexpr size_t buffer_size = 4096;

  while (!finished_ || !pending_.empty())
  {
    jobs.collectReady();
    const size_t newline = pending_.find('\n');

    if (newline != std::string::npos)
    {
      line = pending_.substr(0, newline);
      pending_.erase(0, newline + 1);

      if (!line.empty() && line.back() == '\r')
      {
        line.pop_back();
      }

      return true;
    }

    if (finished_)
    {
      line.swap(pending_);
      pending_.clear();
      return !line.empty();
    }

    std::vector< struct pollfd > descriptors;
    const struct pollfd input = {STDIN_FILENO, POLLIN, 0};
    descriptors.push_back(input);

    for (int result : jobs.resultDescriptors())
    {
      const struct pollfd descriptor = {result, POLLIN, 0};
      descriptors.push_back(descriptor);
    }

    const int ready = poll(descriptors.data(), static_cast< nfds_t >(descriptors.size()), -1);

    if (ready == -1)
    {
      if (errno == EINTR)
      {
        continue;
      }

      throw std::system_error(errno, std::generic_category(), "Cannot wait for commands");
    }

    if ((descriptors[0].revents & POLLNVAL) != 0)
    {
      throw std::runtime_error("Invalid input descriptor");
    }

    if ((descriptors[0].revents & (POLLIN | POLLHUP | POLLERR)) != 0)
    {
      char buffer[buffer_size] = {};
      const ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer));

      if (count == -1)
      {
        if (errno == EINTR)
        {
          continue;
        }

        throw std::system_error(errno, std::generic_category(), "Cannot read commands");
      }

      if (count == 0)
      {
        finished_ = true;
      }
      else
      {
        pending_.append(buffer, static_cast< size_t >(count));
      }
    }
  }

  return false;
}
