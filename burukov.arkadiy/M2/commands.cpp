#include "commands.hpp"

#include <cctype>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../common/shapes.hpp"

#include "ipc.hpp"
#include "worker.hpp"

namespace
{
  std::vector< std::string > splitTokens(const std::string &line)
  {
    std::vector< std::string > tokens;
    std::size_t i = 0;
    const std::size_t len = line.length();
    while (i < len)
    {
      while (i < len && std::isspace(static_cast< unsigned char >(line[i])) != 0)
      {
        ++i;
      }
      if (i >= len)
      {
        break;
      }
      const std::size_t start = i;
      while (i < len && std::isspace(static_cast< unsigned char >(line[i])) == 0)
      {
        ++i;
      }
      tokens.push_back(line.substr(start, i - start));
    }
    return tokens;
  }

  bool isValidIdentifier(const std::string &name) noexcept
  {
    if (name.empty())
    {
      return false;
    }
    const char first = name[0];
    const bool is_upper = (first >= 'A' && first <= 'Z');
    const bool is_lower = (first >= 'a' && first <= 'z');
    return is_upper || is_lower;
  }

  bool parseLongLong(const std::string &str, long long &out) noexcept
  {
    if (str.empty())
    {
      return false;
    }
    try
    {
      std::size_t pos = 0;
      const long long val = std::stoll(str, &pos);
      if (pos != str.length())
      {
        return false;
      }
      out = val;
      return true;
    }
    catch (const std::exception &)
    {
      return false;
    }
  }

  bool parseDouble(const std::string &str, double &out) noexcept
  {
    if (str.empty())
    {
      return false;
    }
    try
    {
      std::size_t pos = 0;
      const double val = std::stod(str, &pos);
      if (pos != str.length() || std::isnan(val) || std::isinf(val))
      {
        return false;
      }
      out = val;
      return true;
    }
    catch (const std::exception &)
    {
      return false;
    }
  }

  void printFrameRect(burukov::rectangle_t rect)
  {
    const long long min_x = static_cast< long long >(std::floor(rect.min_x));
    const long long min_y = static_cast< long long >(std::floor(rect.min_y));
    const long long max_x = static_cast< long long >(std::ceil(rect.max_x));
    const long long max_y = static_cast< long long >(std::ceil(rect.max_y));
    std::cout << "(" << min_x << " " << min_y << ") (" << max_x << " " << max_y << ")\n";
  }
}

burukov::CommandProcessor::CommandProcessor() = default;

burukov::CommandProcessor::~CommandProcessor() noexcept
{
  shutdownAll();
}

void burukov::CommandProcessor::shutdownAll() noexcept
{
  for (auto it = processes_.begin(); it != processes_.end(); ++it)
  {
    worker_process_t &proc = it->second;
    if (proc.to_child.get() >= 0)
    {
      sendStopSignal(proc.to_child.get());
      proc.to_child.closeFd();
    }
    proc.from_child.closeFd();
    if (proc.pid > 0)
    {
      int status = 0;
      waitpid(proc.pid, &status, 0);
      proc.pid = -1;
    }
  }
}

void burukov::CommandProcessor::dispatchNextTask(worker_process_t &proc) noexcept
{
  if (proc.busy || proc.killed || proc.queue.empty() || proc.to_child.get() < 0)
  {
    return;
  }

  const pending_task_t task = std::move(proc.queue.front());
  proc.queue.pop_front();

  if (sendTask(proc.to_child.get(), task.calc_id, task.threads, task.tries, task.shapes))
  {
    proc.busy = true;
    proc.current_calc_id = task.calc_id;
  }
  else
  {
    calcs_[task.calc_id].state = calc_state_t::canceled;
  }
}

void burukov::CommandProcessor::consumeResult(worker_process_t &proc, const task_result_t &res) noexcept
{
  if (res.calc_id < calcs_.size())
  {
    calc_record_t &rec = calcs_[res.calc_id];
    rec.duration_ms = res.duration_ms;
    rec.area = res.area;
    rec.state = calc_state_t::completed;
  }
  proc.busy = false;

  if (proc.killed)
  {
    proc.to_child.closeFd();
    proc.from_child.closeFd();
    if (proc.pid > 0)
    {
      int status = 0;
      waitpid(proc.pid, &status, 0);
      proc.pid = -1;
    }
  }
  else
  {
    dispatchNextTask(proc);
  }
}

void burukov::CommandProcessor::pollWorkers() noexcept
{
  for (auto it = processes_.begin(); it != processes_.end(); ++it)
  {
    worker_process_t &proc = it->second;
    while (proc.busy && proc.from_child.get() >= 0)
    {
      pollfd pfd = {proc.from_child.get(), POLLIN, 0};
      const int ret = poll(&pfd, 1, 0);
      if (ret > 0 && ((pfd.revents & (POLLIN | POLLHUP)) != 0))
      {
        task_result_t res = {0, 0.0, 0.0};
        if (readAll(proc.from_child.get(), &res, sizeof(res)))
        {
          consumeResult(proc, res);
        }
        else
        {
          proc.busy = false;
          proc.from_child.closeFd();
          break;
        }
      }
      else
      {
        break;
      }
    }
  }
}

bool burukov::CommandProcessor::waitForCalculation(std::size_t calc_id, long long timeout_ms) noexcept
{
  pollWorkers();
  if (calcs_[calc_id].state != calc_state_t::in_progress)
  {
    return true;
  }

  const std::string &proc_name = calcs_[calc_id].process_name;
  auto proc_it = processes_.find(proc_name);
  if (proc_it == processes_.end())
  {
    return false;
  }
  worker_process_t &proc = proc_it->second;

  const auto start = std::chrono::steady_clock::now();

  while (calcs_[calc_id].state == calc_state_t::in_progress && proc.from_child.get() >= 0)
  {
    int poll_timeout = -1;
    if (timeout_ms >= 0)
    {
      const auto now = std::chrono::steady_clock::now();
      const auto elapsed_ms = std::chrono::duration_cast< std::chrono::milliseconds >(now - start).count();
      if (elapsed_ms >= timeout_ms)
      {
        return false;
      }
      poll_timeout = static_cast< int >(timeout_ms - elapsed_ms);
    }

    pollfd pfd = {proc.from_child.get(), POLLIN, 0};
    const int ret = poll(&pfd, 1, poll_timeout);
    if (ret == 0)
    {
      return false;
    }
    if (ret < 0)
    {
      continue;
    }
    if ((pfd.revents & (POLLIN | POLLHUP)) != 0)
    {
      task_result_t res = {0, 0.0, 0.0};
      if (readAll(proc.from_child.get(), &res, sizeof(res)))
      {
        consumeResult(proc, res);
      }
      else
      {
        proc.busy = false;
        proc.from_child.closeFd();
        return false;
      }
    }
  }
  return calcs_[calc_id].state != calc_state_t::in_progress;
}

void burukov::CommandProcessor::printCalcStatus(const calc_record_t &rec) const
{
  if (rec.state == calc_state_t::in_progress)
  {
    std::cout << rec.process_name << " <IN PROGRESS>\n";
  }
  else if (rec.state == calc_state_t::canceled)
  {
    std::cout << rec.process_name << " <CANCELED>\n";
  }
  else
  {
    const std::ios::fmtflags old_flags = std::cout.flags();
    const std::streamsize old_prec = std::cout.precision();
    std::cout << rec.process_name << " " << std::fixed << std::setprecision(3) << rec.duration_ms << " " << rec.area
              << '\n';
    std::cout.flags(old_flags);
    std::cout.precision(old_prec);
  }
}

bool burukov::CommandProcessor::handleCircle(const std::vector< std::string > &tokens)
{
  constexpr std::size_t EXPECTED_TOKENS = 5;
  if (tokens.size() != EXPECTED_TOKENS)
  {
    return false;
  }
  const std::string &name = tokens[1];
  long long r = 0;
  long long x = 0;
  long long y = 0;
  if (!isValidIdentifier(name) || shapes_.find(name) != shapes_.end())
  {
    return false;
  }
  if (!parseLongLong(tokens[2], r) || !parseLongLong(tokens[3], x) || !parseLongLong(tokens[4], y) || r <= 0)
  {
    return false;
  }
  shapes_[name]
      = std::make_unique< Circle >(static_cast< double >(r), static_cast< double >(x), static_cast< double >(y));
  return true;
}

bool burukov::CommandProcessor::handleEllipse(const std::vector< std::string > &tokens)
{
  constexpr std::size_t EXPECTED_TOKENS = 6;
  constexpr std::size_t IDX_Y = 5;
  if (tokens.size() != EXPECTED_TOKENS)
  {
    return false;
  }
  const std::string &name = tokens[1];
  long long rx = 0;
  long long ry = 0;
  long long x = 0;
  long long y = 0;
  if (!isValidIdentifier(name) || shapes_.find(name) != shapes_.end())
  {
    return false;
  }
  if (!parseLongLong(tokens[2], rx) || !parseLongLong(tokens[3], ry) || !parseLongLong(tokens[4], x)
      || !parseLongLong(tokens[IDX_Y], y) || rx <= 0 || ry <= 0)
  {
    return false;
  }
  shapes_[name] = std::make_unique< Ellipse >(
      static_cast< double >(rx), static_cast< double >(ry), static_cast< double >(x), static_cast< double >(y));
  return true;
}

bool burukov::CommandProcessor::handleRotate(const std::vector< std::string > &tokens)
{
  if (tokens.size() != 3)
  {
    return false;
  }
  auto it = shapes_.find(tokens[1]);
  double ksi = 0.0;
  if (it == shapes_.end() || !parseDouble(tokens[2], ksi))
  {
    return false;
  }
  it->second->rotate(ksi);
  return true;
}

bool burukov::CommandProcessor::handleSet(const std::vector< std::string > &tokens)
{
  if (tokens.size() < 4)
  {
    return false;
  }
  const std::string &set_name = tokens[1];
  long long set_size = 0;
  if (!isValidIdentifier(set_name) || sets_.find(set_name) != sets_.end() || !parseLongLong(tokens[2], set_size)
      || set_size <= 0)
  {
    return false;
  }
  if (tokens.size() != static_cast< std::size_t >(set_size) + 3)
  {
    return false;
  }

  std::vector< std::string > member_names;
  member_names.reserve(static_cast< std::size_t >(set_size));
  for (std::size_t i = 3; i < tokens.size(); ++i)
  {
    if (shapes_.find(tokens[i]) == shapes_.end())
    {
      return false;
    }
    member_names.push_back(tokens[i]);
  }
  sets_[set_name] = std::move(member_names);
  return true;
}

bool burukov::CommandProcessor::handleShow(const std::vector< std::string > &tokens) const
{
  if (tokens.size() != 2)
  {
    return false;
  }
  auto it = shapes_.find(tokens[1]);
  if (it == shapes_.end())
  {
    return false;
  }
  it->second->print();
  return true;
}

bool burukov::CommandProcessor::handleShowSet(const std::vector< std::string > &tokens) const
{
  if (tokens.size() != 2)
  {
    return false;
  }
  auto it = sets_.find(tokens[1]);
  if (it == sets_.end())
  {
    return false;
  }
  const std::vector< std::string > &members = it->second;
  for (std::size_t i = 0; i < members.size(); ++i)
  {
    auto shape_it = shapes_.find(members[i]);
    if (shape_it != shapes_.end())
    {
      shape_it->second->print();
    }
  }
  return true;
}

bool burukov::CommandProcessor::handleFrame(const std::vector< std::string > &tokens) const
{
  if (tokens.size() != 2)
  {
    return false;
  }
  auto it = shapes_.find(tokens[1]);
  if (it == shapes_.end())
  {
    return false;
  }
  const rectangle_t rect = {it->second->getMinX(), it->second->getMinY(), it->second->getMaxX(), it->second->getMaxY()};
  printFrameRect(rect);
  return true;
}

bool burukov::CommandProcessor::handleFrameSet(const std::vector< std::string > &tokens) const
{
  if (tokens.size() != 2)
  {
    return false;
  }
  auto it = sets_.find(tokens[1]);
  if (it == sets_.end() || it->second.empty())
  {
    return false;
  }
  const std::vector< std::string > &members = it->second;
  std::vector< shape_data_t > serialized;
  serialized.reserve(members.size());
  for (std::size_t i = 0; i < members.size(); ++i)
  {
    auto shape_it = shapes_.find(members[i]);
    if (shape_it != shapes_.end())
    {
      serialized.push_back(shape_it->second->serialize());
    }
  }
  const rectangle_t rect = findDataRange(serialized);
  printFrameRect(rect);
  return true;
}

bool burukov::CommandProcessor::handleSpawn(const std::vector< std::string > &tokens, int &child_exit_flag)
{
  if (tokens.size() != 3)
  {
    return false;
  }
  const std::string &proc_name = tokens[1];
  long long seed = 0;
  if (!isValidIdentifier(proc_name) || processes_.find(proc_name) != processes_.end() || !parseLongLong(tokens[2], seed)
      || seed < 0)
  {
    return false;
  }

  int parent_to_child[2] = {-1, -1};
  int child_to_parent[2] = {-1, -1};
  if (pipe(parent_to_child) != 0)
  {
    return false;
  }
  if (pipe(child_to_parent) != 0)
  {
    close(parent_to_child[0]);
    close(parent_to_child[1]);
    return false;
  }

  std::cout.flush();
  std::cerr.flush();

  const pid_t pid = fork();
  if (pid < 0)
  {
    close(parent_to_child[0]);
    close(parent_to_child[1]);
    close(child_to_parent[0]);
    close(child_to_parent[1]);
    return false;
  }

  if (pid == 0)
  {
    close(parent_to_child[1]);
    close(child_to_parent[0]);
    for (auto it = processes_.begin(); it != processes_.end(); ++it)
    {
      it->second.to_child.closeFd();
      it->second.from_child.closeFd();
    }
    runWorkerLoop(parent_to_child[0], child_to_parent[1], seed);
    close(parent_to_child[0]);
    close(child_to_parent[1]);
    child_exit_flag = 1;
    return true;
  }

  close(parent_to_child[0]);
  close(child_to_parent[1]);

  worker_process_t proc = {pid, seed, PipeEndpoint(parent_to_child[1]), PipeEndpoint(child_to_parent[0]), false, false,
      0, std::deque< pending_task_t >()};
  processes_.emplace(proc_name, std::move(proc));
  return true;
}

bool burukov::CommandProcessor::handleAreaOn(const std::vector< std::string > &tokens)
{
  constexpr std::size_t EXPECTED_TOKENS = 6;
  constexpr std::size_t IDX_TRIES = 5;
  if (tokens.size() != EXPECTED_TOKENS)
  {
    return false;
  }
  const std::string &proc_name = tokens[1];
  const std::string &calc_name = tokens[2];
  const std::string &set_name = tokens[3];
  long long threads = 0;
  long long tries = 0;

  if (!isValidIdentifier(proc_name) || !isValidIdentifier(calc_name) || !isValidIdentifier(set_name))
  {
    return false;
  }
  if (!parseLongLong(tokens[4], threads) || !parseLongLong(tokens[IDX_TRIES], tries) || threads <= 0 || tries <= 0)
  {
    return false;
  }

  pollWorkers();

  auto proc_it = processes_.find(proc_name);
  auto set_it = sets_.find(set_name);
  if (proc_it == processes_.end() || proc_it->second.killed || set_it == sets_.end()
      || calc_index_by_name_.find(calc_name) != calc_index_by_name_.end())
  {
    return false;
  }

  std::vector< shape_data_t > snapshot;
  const std::vector< std::string > &member_names = set_it->second;
  snapshot.reserve(member_names.size());
  for (std::size_t i = 0; i < member_names.size(); ++i)
  {
    auto shape_it = shapes_.find(member_names[i]);
    if (shape_it != shapes_.end())
    {
      snapshot.push_back(shape_it->second->serialize());
    }
  }

  const std::size_t calc_id = calcs_.size();
  calcs_.push_back({calc_id, proc_name, calc_state_t::in_progress, 0.0, 0.0});
  calc_index_by_name_[calc_name] = calc_id;

  proc_it->second.queue.push_back({calc_id, threads, tries, std::move(snapshot)});
  dispatchNextTask(proc_it->second);
  return true;
}

bool burukov::CommandProcessor::handleStatus(const std::vector< std::string > &tokens)
{
  if (tokens.size() != 2)
  {
    return false;
  }
  pollWorkers();
  auto it = calc_index_by_name_.find(tokens[1]);
  if (it == calc_index_by_name_.end())
  {
    return false;
  }
  printCalcStatus(calcs_[it->second]);
  return true;
}

bool burukov::CommandProcessor::handleWait(const std::vector< std::string > &tokens)
{
  if (tokens.size() != 2)
  {
    return false;
  }
  auto it = calc_index_by_name_.find(tokens[1]);
  if (it == calc_index_by_name_.end())
  {
    return false;
  }
  waitForCalculation(it->second, -1);
  printCalcStatus(calcs_[it->second]);
  return true;
}

bool burukov::CommandProcessor::handleTimeout(const std::vector< std::string > &tokens)
{
  if (tokens.size() != 3)
  {
    return false;
  }
  auto it = calc_index_by_name_.find(tokens[1]);
  long long ms = 0;
  if (it == calc_index_by_name_.end() || !parseLongLong(tokens[2], ms) || ms < 0)
  {
    return false;
  }
  waitForCalculation(it->second, ms);
  printCalcStatus(calcs_[it->second]);
  return true;
}

bool burukov::CommandProcessor::handleKill(const std::vector< std::string > &tokens)
{
  if (tokens.size() != 2)
  {
    return false;
  }
  pollWorkers();
  auto it = processes_.find(tokens[1]);
  if (it == processes_.end() || it->second.killed)
  {
    return false;
  }

  worker_process_t &proc = it->second;
  proc.killed = true;

  while (!proc.queue.empty())
  {
    const std::size_t cid = proc.queue.front().calc_id;
    calcs_[cid].state = calc_state_t::canceled;
    proc.queue.pop_front();
  }

  if (proc.to_child.get() >= 0)
  {
    sendStopSignal(proc.to_child.get());
    proc.to_child.closeFd();
  }

  if (proc.busy)
  {
    waitForCalculation(proc.current_calc_id, -1);
  }
  else
  {
    proc.from_child.closeFd();
    if (proc.pid > 0)
    {
      int status = 0;
      waitpid(proc.pid, &status, 0);
      proc.pid = -1;
    }
  }
  return true;
}

bool burukov::CommandProcessor::processLine(const std::string &line, int &child_exit_flag)
{
  const std::vector< std::string > tokens = splitTokens(line);
  if (tokens.empty())
  {
    return true;
  }

  const std::string &cmd = tokens[0];
  bool valid_args = false;

  if (cmd == "circle")
  {
    valid_args = handleCircle(tokens);
  }
  else if (cmd == "ellipse")
  {
    valid_args = handleEllipse(tokens);
  }
  else if (cmd == "rotate")
  {
    valid_args = handleRotate(tokens);
  }
  else if (cmd == "set")
  {
    valid_args = handleSet(tokens);
  }
  else if (cmd == "show")
  {
    valid_args = handleShow(tokens);
  }
  else if (cmd == "showset")
  {
    valid_args = handleShowSet(tokens);
  }
  else if (cmd == "frame")
  {
    valid_args = handleFrame(tokens);
  }
  else if (cmd == "frameset")
  {
    valid_args = handleFrameSet(tokens);
  }
  else if (cmd == "spawn")
  {
    valid_args = handleSpawn(tokens, child_exit_flag);
    if (child_exit_flag != 0)
    {
      return true;
    }
  }
  else if (cmd == "area-on")
  {
    valid_args = handleAreaOn(tokens);
  }
  else if (cmd == "status")
  {
    valid_args = handleStatus(tokens);
  }
  else if (cmd == "wait")
  {
    valid_args = handleWait(tokens);
  }
  else if (cmd == "timeout")
  {
    valid_args = handleTimeout(tokens);
  }
  else if (cmd == "kill")
  {
    valid_args = handleKill(tokens);
  }
  else
  {
    std::cout << "<UNKNOWN COMMAND>\n";
    return true;
  }

  if (!valid_args)
  {
    std::cout << "<INVALID COMMAND>\n";
  }
  return true;
}
