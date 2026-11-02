#ifndef IPC_HPP
#define IPC_HPP

#include <cstddef>
#include <vector>

#include "../common/shapes.hpp"

namespace burukov
{
  struct task_header_t
  {
    int type;
    std::size_t calc_id;
    long long threads;
    long long tries;
    std::size_t shape_count;
  };

  struct task_result_t
  {
    std::size_t calc_id;
    double duration_ms;
    double area;
  };

  class PipeEndpoint
  {
  public:
    PipeEndpoint() noexcept;
    explicit PipeEndpoint(int fd) noexcept;
    PipeEndpoint(const PipeEndpoint &) = delete;
    PipeEndpoint(PipeEndpoint &&other) noexcept;
    ~PipeEndpoint() noexcept;

    PipeEndpoint &operator=(const PipeEndpoint &) = delete;
    PipeEndpoint &operator=(PipeEndpoint &&other) noexcept;

    int get() const noexcept;
    void closeFd() noexcept;

  private:
    int fd_;
  };

  bool writeAll(int fd, const void *buffer, std::size_t count) noexcept;
  bool readAll(int fd, void *buffer, std::size_t count) noexcept;

  bool sendTask(int fd, std::size_t calc_id, long long threads, long long tries,
      const std::vector< shape_data_t > &shapes) noexcept;
  bool sendStopSignal(int fd) noexcept;
}

#endif
