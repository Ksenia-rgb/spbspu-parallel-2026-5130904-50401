#include "jobs.hpp"
#include <cerrno>
#include <cstddef>
#include <algorithm>
#include <chrono>
#include <exception>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <poll.h>
#include "channel.hpp"
#include "input.hpp"
#include "protocol.hpp"

kondrat::Jobs::Jobs(ProcessMap & processes):
  processes_(processes),
  jobs_(),
  queues_(),
  failed_processes_(),
  next_id_(1)
{}

void kondrat::Jobs::submit(
  const std::vector< std::string > & words,
  const std::map< std::string, Shape > & shapes,
  const std::map< std::string, std::vector< std::string > > & sets)
{
  constexpr size_t expected_words = 6;
  constexpr size_t name_index = 2;
  constexpr size_t set_index = 3;
  constexpr size_t threads_index = 4;
  constexpr size_t tries_index = 5;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::string & process_name = words[1];
  const std::string & name = words[name_index];
  const std::map< std::string, std::vector< std::string > >::const_iterator set = sets.find(words[set_index]);

  if (processes_.find(process_name) == processes_.end() || failed_processes_.count(process_name) != 0)
  {
    throw std::invalid_argument("Unknown or unavailable process");
  }

  if (!isValidName(name) || jobs_.find(name) != jobs_.end() || set == sets.end())
  {
    throw std::invalid_argument("Invalid calculation name or set");
  }

  const long long threads = parseLongLong(words[threads_index]);
  const long long tries = parseLongLong(words[tries_index]);

  if (threads < 0 || tries <= 0 || next_id_ == std::numeric_limits< unsigned long long >::max())
  {
    throw std::invalid_argument("Invalid calculation parameters");
  }

  std::vector< Shape > snapshot;

  for (const std::string & shape_name : set->second)
  {
    snapshot.push_back(shapes.at(shape_name));
  }

  Job job = {next_id_, process_name, threads, tries, std::move(snapshot), false, JobState::InProgress, 0.0, 0.0};
  jobs_.emplace(name, std::move(job));
  queues_[process_name].push_back(name);
  ++next_id_;
  startNext(process_name);
}

void kondrat::Jobs::startNext(const std::string & process_name)
{
  std::deque< std::string > & queue = queues_.at(process_name);

  if (queue.empty() || failed_processes_.count(process_name) != 0)
  {
    return;
  }

  Job & job = jobs_.at(queue.front());

  if (job.started)
  {
    return;
  }

  const Request request = {RequestKind::Area, job.id, job.threads, job.tries, job.shapes.size()};
  const int descriptor = processes_.at(process_name)->requestDescriptor();

  try
  {
    writeFull(descriptor, &request, sizeof(request));
    writeFull(descriptor, job.shapes.data(), job.shapes.size() * sizeof(Shape));
    job.started = true;
    std::vector< Shape >().swap(job.shapes);
  }
  catch (const std::exception &)
  {
    cancelProcess(process_name);
  }
}

void kondrat::Jobs::cancelProcess(const std::string & process_name)
{
  std::deque< std::string > & queue = queues_.at(process_name);

  for (const std::string & name : queue)
  {
    Job & job = jobs_.at(name);
    job.state = JobState::Canceled;
    std::vector< Shape >().swap(job.shapes);
  }

  queue.clear();
  failed_processes_.insert(process_name);
}

void kondrat::Jobs::receive(const std::string & process_name)
{
  std::deque< std::string > & queue = queues_.at(process_name);

  try
  {
    Response response = {};
    const int descriptor = processes_.at(process_name)->resultDescriptor();

    if (!readFull(descriptor, &response, sizeof(response)))
    {
      cancelProcess(process_name);
      return;
    }

    Job & job = jobs_.at(queue.front());

    if (response.calculation_id != job.id)
    {
      throw std::runtime_error("Unexpected calculation response");
    }

    job.time_ms = response.time_ms;
    job.area = response.area;
    job.state = JobState::Done;
    queue.pop_front();
  }
  catch (const std::exception &)
  {
    cancelProcess(process_name);
    return;
  }

  startNext(process_name);
}

void kondrat::Jobs::collectReady()
{
  for (const std::map< std::string, std::deque< std::string > >::value_type & entry : queues_)
  {
    if (entry.second.empty())
    {
      continue;
    }

    struct pollfd descriptor = {};
    descriptor.fd = processes_.at(entry.first)->resultDescriptor();
    descriptor.events = POLLIN;
    int ready = poll(&descriptor, 1, 0);

    while (ready == -1 && errno == EINTR)
    {
      ready = poll(&descriptor, 1, 0);
    }

    if (ready == -1 || (descriptor.revents & (POLLERR | POLLNVAL)) != 0)
    {
      cancelProcess(entry.first);
    }
    else if (ready > 0 && (descriptor.revents & (POLLIN | POLLHUP)) != 0)
    {
      receive(entry.first);
    }
  }
}

void kondrat::Jobs::show(const std::vector< std::string > & words, bool wait, std::ostream & output)
{
  constexpr size_t expected_words = 2;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::map< std::string, Job >::iterator position = jobs_.find(words[1]);

  if (position == jobs_.end())
  {
    throw std::invalid_argument("Unknown calculation");
  }

  collectReady();
  Job & job = position->second;

  while (wait && job.state == JobState::InProgress)
  {
    receive(job.process_name);
  }

  printResult(job, output);
}

void kondrat::Jobs::printResult(const Job & job, std::ostream & output)
{
  constexpr int result_precision = 3;

  output << job.process_name << ' ';

  if (job.state == JobState::Done)
  {
    output << std::fixed << std::setprecision(result_precision) << job.time_ms << ' ' << job.area << '\n';
  }
  else if (job.state == JobState::Canceled)
  {
    output << "<CANCELED>\n";
  }
  else
  {
    output << "<IN PROGRESS>\n";
  }
}

void kondrat::Jobs::finishAll()
{
  for (const std::map< std::string, std::deque< std::string > >::value_type & entry : queues_)
  {
    while (!entry.second.empty())
    {
      receive(entry.first);
    }
  }
}

std::vector< int > kondrat::Jobs::resultDescriptors() const
{
  std::vector< int > descriptors;

  for (const std::map< std::string, std::deque< std::string > >::value_type & entry : queues_)
  {
    if (!entry.second.empty())
    {
      descriptors.push_back(processes_.at(entry.first)->resultDescriptor());
    }
  }

  return descriptors;
}

void kondrat::Jobs::timeout(const std::vector< std::string > & words, std::ostream & output)
{
  constexpr size_t expected_words = 3;
  constexpr size_t milliseconds_index = 2;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const long long milliseconds = parseLongLong(words[milliseconds_index]);
  const std::map< std::string, Job >::iterator position = jobs_.find(words[1]);

  if (milliseconds < 0 || position == jobs_.end())
  {
    throw std::invalid_argument("Invalid timeout or calculation name");
  }

  const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  collectReady();
  Job & job = position->second;

  while (job.state == JobState::InProgress)
  {
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    const long long elapsed = std::chrono::duration_cast< std::chrono::milliseconds >(now - start).count();

    if (elapsed >= milliseconds)
    {
      break;
    }

    const long long remaining = milliseconds - elapsed;
    const long long interval = std::min< long long >(remaining, std::numeric_limits< int >::max());
    struct pollfd descriptor = {};
    descriptor.fd = processes_.at(job.process_name)->resultDescriptor();
    descriptor.events = POLLIN;
    const int ready = poll(&descriptor, 1, static_cast< int >(interval));

    if (ready == -1 && errno == EINTR)
    {
      continue;
    }

    if (ready == -1 || (descriptor.revents & (POLLERR | POLLNVAL)) != 0)
    {
      cancelProcess(job.process_name);
      break;
    }

    if (ready > 0 && (descriptor.revents & (POLLIN | POLLHUP)) != 0)
    {
      receive(job.process_name);
    }
  }

  printResult(job, output);
}

void kondrat::Jobs::stopProcess(const std::vector< std::string > & words)
{
  constexpr size_t expected_words = 2;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::string & process_name = words[1];
  const ProcessMap::iterator position = processes_.find(process_name);

  if (position == processes_.end() || failed_processes_.count(process_name) != 0)
  {
    throw std::invalid_argument("Unknown or unavailable process");
  }

  failed_processes_.insert(process_name);
  std::deque< std::string > & queue = queues_[process_name];

  while (queue.size() > 1)
  {
    Job & job = jobs_.at(queue.back());
    job.state = JobState::Canceled;
    std::vector< Shape >().swap(job.shapes);
    queue.pop_back();
  }

  if (!queue.empty())
  {
    receive(process_name);
  }

  position->second->stop();
}
