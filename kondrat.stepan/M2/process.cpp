#include "process.hpp"
#include <cerrno>
#include <cstddef>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <sys/wait.h>
#include <unistd.h>
#include "input.hpp"
#include "worker.hpp"

kondrat::Process::Process(long long seed, const std::vector< int > & inherited_descriptors):
  pid_(-1),
  request_descriptor_(-1),
  result_descriptor_(-1)
{
  int requests[2] = {-1, -1};
  int results[2] = {-1, -1};

  if (pipe(requests) == -1)
  {
    throw std::system_error(errno, std::generic_category(), "Cannot create request pipe");
  }

  if (pipe(results) == -1)
  {
    const int error = errno;

    close(requests[0]);
    close(requests[1]);
    throw std::system_error(error, std::generic_category(), "Cannot create result pipe");
  }

  pid_ = fork();

  if (pid_ == -1)
  {
    const int error = errno;

    close(requests[0]);
    close(requests[1]);
    close(results[0]);
    close(results[1]);
    throw std::system_error(error, std::generic_category(), "Cannot create process");
  }

  if (pid_ == 0)
  {
    close(requests[1]);
    close(results[0]);

    for (int descriptor : inherited_descriptors)
    {
      close(descriptor);
    }

    const int exit_code = workerLoop(requests[0], results[1], seed);
    _exit(exit_code);
  }

  close(requests[0]);
  close(results[1]);
  request_descriptor_ = requests[1];
  result_descriptor_ = results[0];
}

kondrat::Process::~Process()
{
  stop();
}

void kondrat::Process::stop()
{
  if (request_descriptor_ != -1)
  {
    close(request_descriptor_);
    request_descriptor_ = -1;
  }

  if (result_descriptor_ != -1)
  {
    close(result_descriptor_);
    result_descriptor_ = -1;
  }

  if (pid_ != -1)
  {
    while (waitpid(pid_, nullptr, 0) == -1 && errno == EINTR)
    {
    }

    pid_ = -1;
  }
}

int kondrat::Process::requestDescriptor() const
{
  return request_descriptor_;
}

int kondrat::Process::resultDescriptor() const
{
  return result_descriptor_;
}

void kondrat::spawnProcess(const std::vector< std::string > & words, ProcessMap & processes)
{
  constexpr size_t expected_words = 3;
  constexpr size_t seed_index = 2;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::string & name = words[1];

  if (!isValidName(name) || processes.find(name) != processes.end())
  {
    throw std::invalid_argument("Invalid or duplicate process name");
  }

  const long long seed = parseLongLong(words[seed_index]);

  if (seed < 0)
  {
    throw std::invalid_argument("Seed cannot be negative");
  }

  std::vector< int > inherited_descriptors;

  for (const ProcessMap::value_type & entry : processes)
  {
    const int request_descriptor = entry.second->requestDescriptor();
    const int result_descriptor = entry.second->resultDescriptor();

    if (request_descriptor != -1)
    {
      inherited_descriptors.push_back(request_descriptor);
    }

    if (result_descriptor != -1)
    {
      inherited_descriptors.push_back(result_descriptor);
    }
  }

  std::unique_ptr< Process > process(new Process(seed, inherited_descriptors));
  processes.emplace(name, std::move(process));
}
