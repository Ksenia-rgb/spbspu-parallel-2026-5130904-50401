#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <cstddef>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <sys/types.h>

#include "../common/shapes.hpp"

#include "ipc.hpp"

namespace burukov
{
  enum class calc_state_t
  {
    in_progress,
    completed,
    canceled
  };

  struct pending_task_t
  {
    std::size_t calc_id;
    long long threads;
    long long tries;
    std::vector< shape_data_t > shapes;
  };

  struct calc_record_t
  {
    std::size_t id;
    std::string process_name;
    calc_state_t state;
    double duration_ms;
    double area;
  };

  struct worker_process_t
  {
    pid_t pid;
    long long seed;
    PipeEndpoint to_child;
    PipeEndpoint from_child;
    bool killed;
    bool busy;
    std::size_t current_calc_id;
    std::deque< pending_task_t > queue;
  };

  class CommandProcessor
  {
  public:
    CommandProcessor();
    CommandProcessor(const CommandProcessor &) = delete;
    CommandProcessor(CommandProcessor &&) = delete;
    ~CommandProcessor() noexcept;

    CommandProcessor &operator=(const CommandProcessor &) = delete;
    CommandProcessor &operator=(CommandProcessor &&) = delete;

    bool processLine(const std::string &line, int &child_exit_flag);
    void shutdownAll() noexcept;

  private:
    std::map< std::string, std::unique_ptr< Shape > > shapes_;
    std::map< std::string, std::vector< std::string > > sets_;
    std::map< std::string, worker_process_t > processes_;
    std::map< std::string, std::size_t > calc_index_by_name_;
    std::vector< calc_record_t > calcs_;

    void pollWorkers() noexcept;
    bool waitForCalculation(std::size_t calc_id, long long timeout_ms) noexcept;
    void dispatchNextTask(worker_process_t &proc) noexcept;
    void consumeResult(worker_process_t &proc, const task_result_t &res) noexcept;
    void printCalcStatus(const calc_record_t &rec) const;

    bool handleCircle(const std::vector< std::string > &tokens);
    bool handleEllipse(const std::vector< std::string > &tokens);
    bool handleRotate(const std::vector< std::string > &tokens);
    bool handleSet(const std::vector< std::string > &tokens);
    bool handleShow(const std::vector< std::string > &tokens) const;
    bool handleShowSet(const std::vector< std::string > &tokens) const;
    bool handleFrame(const std::vector< std::string > &tokens) const;
    bool handleFrameSet(const std::vector< std::string > &tokens) const;
    bool handleSpawn(const std::vector< std::string > &tokens, int &child_exit_flag);
    bool handleAreaOn(const std::vector< std::string > &tokens);
    bool handleStatus(const std::vector< std::string > &tokens);
    bool handleWait(const std::vector< std::string > &tokens);
    bool handleTimeout(const std::vector< std::string > &tokens);
    bool handleKill(const std::vector< std::string > &tokens);
  };
}

#endif
