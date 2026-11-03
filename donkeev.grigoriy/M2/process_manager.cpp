#include "process_manager.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <cerrno>
#include <random>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>

namespace
{
  struct Result
  {
    long long in_union = 0;
    long long in_intersection = 0;
  };

  struct Bounds
  {
    double min_x;
    double max_x;
    double min_y;
    double max_y;
  };

  bool contains(const donkeev::circle_t& c, donkeev::point_t pt)
  {
    donkeev::point_t center = c.getCenter();
    double dx = pt.x_ - center.x_;
    double dy = pt.y_ - center.y_;
    double r = c.getRadius();
    return (dx * dx + dy * dy) <= (r * r);
  }

  Result worker(int id, long long my_tries, const std::vector<donkeev::circle_t>& circles, const Bounds& b, long long seed)
  {
    std::default_random_engine gen(seed + static_cast<unsigned>(id));

    std::uniform_real_distribution<double> dist_x(b.min_x, b.max_x);
    std::uniform_real_distribution<double> dist_y(b.min_y, b.max_y);

    Result res;
    for (long long i = 0; i < my_tries; ++i)
    {
      const double x = dist_x(gen);
      const double y = dist_y(gen);

      bool in_any = false;
      bool in_all = true;
      for (const auto& c : circles)
      {
        const bool inside = contains(c, {static_cast<int>(x), static_cast<int>(y)});
        in_any = in_any || inside;
        in_all = in_all && inside;
      }

      if (in_any)
      {
        res.in_union++;
      }
      if (in_all)
      {
        res.in_intersection++;
      }
    }
    return res;
  }

  bool writeAll(int fd, const void* buffer, size_t count)
  {
    const char* ptr = static_cast<const char*>(buffer);
    size_t total_written = 0;
    while (total_written < count)
    {
      ssize_t written = write(fd, ptr + total_written, count - total_written);
      if (written < 0)
      {
        if (errno == EINTR) continue;
        return false;
      }
      total_written += static_cast<size_t>(written);
    }
    return true;
  }

  bool readAll(int fd, void* buffer, size_t count)
  {
    char* ptr = static_cast<char*>(buffer);
    size_t total_read = 0;
    while (total_read < count)
    {
      ssize_t bytes_read = read(fd, ptr + total_read, count - total_read);
      if (bytes_read < 0)
      {
        if (errno == EINTR) continue;
        return false;
      }
      if (bytes_read == 0) return false;
      total_read += static_cast<size_t>(bytes_read);
    }
    return true;
  }

  struct RawCircleData
  {
    int r;
    int x;
    int y;
  };

  struct RawResultData
  {
    double area;
    double time_ms;
  };

  void runChildLoop(int read_fd, int write_fd, size_t seed)
  {
    std::mt19937_64 master_rng(seed);

    while (true)
    {
      size_t num_circles = 0;
      if (!readAll(read_fd, &num_circles, sizeof(num_circles)))
      {
        break;
      }

      std::vector<donkeev::circle_t> circles;
      circles.reserve(num_circles);
      for (size_t i = 0; i < num_circles; ++i)
      {
        RawCircleData raw{};
        if (!readAll(read_fd, &raw, sizeof(raw))) return;
        circles.emplace_back(raw.r, raw.x, raw.y);
      }

      size_t threads_count = 0;
      size_t tries = 0;
      if (!readAll(read_fd, &threads_count, sizeof(threads_count)) ||
          !readAll(read_fd, &tries, sizeof(tries)))
      {
        return;
      }

      auto start_time = std::chrono::high_resolution_clock::now();

      donkeev::rectangle_t bbox = circles[0].getBoundingBox();
      for (size_t i = 1; i < circles.size(); ++i)
      {
        donkeev::rectangle_t curr = circles[i].getBoundingBox();
        if (curr.bottom_left_.x_ < bbox.bottom_left_.x_) bbox.bottom_left_.x_ = curr.bottom_left_.x_;
        if (curr.bottom_left_.y_ < bbox.bottom_left_.y_) bbox.bottom_left_.y_ = bbox.bottom_left_.y_;
        if (curr.top_right_.x_ > bbox.top_right_.x_) bbox.top_right_.x_ = curr.top_right_.x_;
        if (curr.top_right_.y_ > bbox.top_right_.y_) bbox.top_right_.y_ = curr.top_right_.y_;
      }

      Bounds b{
        static_cast<double>(bbox.bottom_left_.x_),
        static_cast<double>(bbox.top_right_.x_),
        static_cast<double>(bbox.bottom_left_.y_),
        static_cast<double>(bbox.top_right_.y_)
      };

      double box_area = (b.max_x - b.min_x) * (b.max_y - b.min_y);

      std::vector<std::thread> workers;
      std::vector<Result> thread_results(threads_count);
      workers.reserve(threads_count);

      long long base_tries = static_cast<long long>(tries / threads_count);
      long long remainder = static_cast<long long>(tries % threads_count);

      for (int i = 0; i < static_cast<int>(threads_count); ++i)
      {
        long long my_tries = base_tries + (i < remainder ? 1 : 0);
        long long thread_seed = static_cast<long long>(master_rng());
        
        workers.emplace_back([i, my_tries, &circles, b, thread_seed, &thread_results]() {
          thread_results[i] = worker(i, my_tries, circles, b, thread_seed);
        });
      }

      for (auto& th : workers)
      {
        th.join();
      }

      long long total_union = 0;
      for (const auto& res : thread_results)
      {
        total_union += res.in_union;
      }

      double area = box_area * (static_cast<double>(total_union) / static_cast<double>(tries));

      auto end_time = std::chrono::high_resolution_clock::now();
      double time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

      RawResultData result{area, time_ms};
      if (!writeAll(write_fd, &result, sizeof(result)))
      {
        return;
      }
    }
  }
}

void donkeev::closeProcess(Process& proc)
{
  if (proc.pipe_to_child_[0] != -1)
  {
    close(proc.pipe_to_child_[0]);
    proc.pipe_to_child_[0] = -1;
  }
  if (proc.pipe_to_child_[1] != -1)
  {
    close(proc.pipe_to_child_[1]);
    proc.pipe_to_child_[1] = -1;
  }
  if (proc.pipe_from_child_[0] != -1)
  {
    close(proc.pipe_from_child_[0]);
    proc.pipe_from_child_[0] = -1;
  }
  if (proc.pipe_from_child_[1] != -1)
  {
    close(proc.pipe_from_child_[1]);
    proc.pipe_from_child_[1] = -1;
  }
}

donkeev::Process donkeev::spawnProcess(size_t seed, const std::vector< Process >& existing_processes)
{
  Process proc{-1, {-1, -1}, {-1, -1}};

  if (pipe(proc.pipe_to_child_) == -1)
  {
    throw std::runtime_error("Failed to create pipe_to_child");
  }

  if (pipe(proc.pipe_from_child_) == -1)
  {
    closeProcess(proc);
    throw std::runtime_error("Failed to create pipe_from_child");
  }

  pid_t pid = fork();
  if (pid < 0)
  {
    closeProcess(proc);
    throw std::runtime_error("Failed to fork process");
  }

  if (pid == 0)
  {
    for (auto existing : existing_processes)
    {
      closeProcess(existing);
    }

    close(proc.pipe_to_child_[1]);
    close(proc.pipe_from_child_[0]);

    runChildLoop(proc.pipe_to_child_[0], proc.pipe_from_child_[1], seed);

    close(proc.pipe_to_child_[0]);
    close(proc.pipe_from_child_[1]);
    _exit(0);
  }

  proc.pid_ = pid;
  close(proc.pipe_to_child_[0]);
  proc.pipe_to_child_[0] = -1;
  close(proc.pipe_from_child_[1]);
  proc.pipe_from_child_[1] = -1;

  return proc;
}

donkeev::ProcessManager::~ProcessManager()
{
  for (auto& pair : processes_)
  {
    closeProcess(pair.second);
    if (pair.second.pid_ > 0)
    {
      waitpid(pair.second.pid_, nullptr, 0);
    }
  }
}

void donkeev::ProcessManager::addProcess(const std::string& name, const Process& proc)
{
  processes_.emplace(name, proc);
}

bool donkeev::ProcessManager::hasProcess(const std::string& name) const
{
  return processes_.count(name) > 0;
}

bool donkeev::ProcessManager::hasCalculation(const std::string& calcName) const
{
  return calculations_.count(calcName) > 0;
}

std::vector< donkeev::Process > donkeev::ProcessManager::getAllProcesses() const
{
  std::vector< Process > result;
  result.reserve(processes_.size());
  for (const auto& entry : processes_)
  {
    result.push_back(entry.second);
  }
  return result;
}

void donkeev::ProcessManager::startCalculation(const std::string& processName, const std::string& calcName,
    const std::string& setName, const ShapeManager& shapeManager, size_t threads, size_t tries)
{
  if (calculations_.count(calcName) > 0)
  {
    throw std::invalid_argument("Calculation with this name already exists");
  }

  auto procIt = processes_.find(processName);
  if (procIt == processes_.end())
  {
    throw std::invalid_argument("Process does not exist");
  }

  const auto& shapeSet = shapeManager.getSet(setName);
  const auto& shapeNames = shapeSet.shape_names_;
  size_t num_circles = shapeNames.size();

  int write_fd = procIt->second.pipe_to_child_[1];

  if (!writeAll(write_fd, &num_circles, sizeof(num_circles)))
  {
    throw std::runtime_error("Failed to write to process pipe");
  }

  for (const auto& sName : shapeNames)
  {
    const auto& circle = shapeManager.getCircle(sName);
    point_t center = circle.getCenter();
    RawCircleData raw{circle.getRadius(), center.x_, center.y_};
    if (!writeAll(write_fd, &raw, sizeof(raw)))
    {
      throw std::runtime_error("Failed to write circle data");
    }
  }

  if (!writeAll(write_fd, &threads, sizeof(threads)) || !writeAll(write_fd, &tries, sizeof(tries)))
  {
    throw std::runtime_error("Failed to write calculation parameters");
  }

  calculations_.emplace(calcName, CalculationInfo{processName, false, 0.0, 0.0});
  pending_calcs_[processName].push(calcName);
}

void donkeev::ProcessManager::readOneResultBlocking(const std::string& processName)
{
  auto procIt = processes_.find(processName);
  auto& queue = pending_calcs_[processName];
  if (procIt == processes_.end() || queue.empty())
  {
    return;
  }

  RawResultData raw{};
  int read_fd = procIt->second.pipe_from_child_[0];
  if (!readAll(read_fd, &raw, sizeof(raw)))
  {
    throw std::runtime_error("Failed to read calculation result from child");
  }

  std::string finished_calc_name = queue.front();
  queue.pop();

  auto calcIt = calculations_.find(finished_calc_name);
  if (calcIt != calculations_.end())
  {
    calcIt->second.completed_ = true;
    calcIt->second.area_ = raw.area;
    calcIt->second.time_ms_ = raw.time_ms;
  }
}

void donkeev::ProcessManager::pollProcessResults(const std::string& processName)
{
  auto procIt = processes_.find(processName);
  auto& queue = pending_calcs_[processName];
  if (procIt == processes_.end())
  {
    return;
  }

  int read_fd = procIt->second.pipe_from_child_[0];

  while (!queue.empty())
  {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(read_fd, &read_set);

    timeval timeout{0, 0};
    int ready = select(read_fd + 1, &read_set, nullptr, nullptr, &timeout);
    if (ready > 0 && FD_ISSET(read_fd, &read_set))
    {
      readOneResultBlocking(processName);
    }
    else
    {
      break;
    }
  }
}

void donkeev::ProcessManager::printStatus(const std::string& calcName, std::ostream& out)
{
  auto it = calculations_.find(calcName);
  if (it == calculations_.end())
  {
    throw std::invalid_argument("Calculation was not started");
  }

  if (!it->second.completed_)
  {
    pollProcessResults(it->second.process_name_);
  }

  if (!it->second.completed_)
  {
    out << it->second.process_name_ << " <IN PROGRESS>\n";
  }
  else
  {
    std::ios_base::fmtflags old_flags = out.flags();
    std::streamsize old_prec = out.precision();
    out << std::fixed << std::setprecision(3) << it->second.process_name_ << " " << it->second.time_ms_ << " "
        << it->second.area_ << "\n";
    out.flags(old_flags);
    out.precision(old_prec);
  }
}

void donkeev::ProcessManager::waitCalculation(const std::string& calcName, std::ostream& out)
{
  auto it = calculations_.find(calcName);
  if (it == calculations_.end())
  {
    throw std::invalid_argument("Calculation was not started");
  }

  while (!it->second.completed_)
  {
    readOneResultBlocking(it->second.process_name_);
  }

  std::ios_base::fmtflags old_flags = out.flags();
  std::streamsize old_prec = out.precision();
  out << std::fixed << std::setprecision(3) << it->second.process_name_ << " " << it->second.time_ms_ << " "
      << it->second.area_ << "\n";
  out.flags(old_flags);
  out.precision(old_prec);
}
