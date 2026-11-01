#include "worker.hpp"
#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>
#include <map>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include "montecarlo.hpp"

namespace
{
  struct CalcResult
  {
    double ms;
    double area;
  };

  using Calcs = std::map< size_t, std::shared_future< CalcResult > >;

  CalcResult runCalc(const std::vector< zharov::Circle >& shapes, size_t threads, size_t tries, size_t seed,
      std::shared_future< CalcResult > prev)
  {
    if (prev.valid())
    {
      prev.wait();
    }
    const auto start = std::chrono::steady_clock::now();
    const double area = zharov::calcArea(shapes, threads, tries, seed);
    const auto finish = std::chrono::steady_clock::now();
    const double ms = std::chrono::duration< double, std::milli >(finish - start).count();
    return {ms, area};
  }

  void startCalc(const zharov::Request& req, const std::vector< zharov::Circle >& shapes, size_t seed, Calcs& calcs)
  {
    std::shared_future< CalcResult > prev;
    if (!calcs.empty())
    {
      prev = calcs.rbegin()->second;
    }
    calcs[req.calc_id] = std::async(std::launch::async, runCalc, shapes, req.threads, req.tries, seed, prev).share();
  }

  zharov::Response checkCalc(const zharov::Request& req, const Calcs& calcs)
  {
    const std::shared_future< CalcResult >& calc = calcs.at(req.calc_id);
    const bool is_status = req.kind == zharov::RequestKind::STATUS;
    if (is_status && calc.wait_for(std::chrono::seconds::zero()) != std::future_status::ready)
    {
      return {req.calc_id, zharov::CalcState::PROGRESS, 0.0, 0.0};
    }
    const CalcResult res = calc.get();
    return {req.calc_id, zharov::CalcState::DONE, res.ms, res.area};
  }
}

void zharov::childLoop(int rd, int wr, size_t seed)
{
  Calcs calcs;
  Request req{};
  std::vector< Circle > shapes;
  while (recvRequest(rd, req, shapes) && req.kind != RequestKind::STOP)
  {
    Response res{req.calc_id, CalcState::PROGRESS, 0.0, 0.0};
    if (req.kind == RequestKind::CALC)
    {
      startCalc(req, shapes, seed, calcs);
    }
    else
    {
      res = checkCalc(req, calcs);
    }
    if (!sendResponse(wr, res))
    {
      break;
    }
  }
}

zharov::Worker::Worker(size_t seed, const std::vector< int >& inherited):
  pid_(-1)
{
  int cmd[2]{};
  if (pipe(cmd) < 0)
  {
    throw std::runtime_error("pipe failed");
  }
  std::unique_ptr< Fd > cmd_rd = std::make_unique< Fd >(cmd[0]);
  std::unique_ptr< Fd > cmd_wr = std::make_unique< Fd >(cmd[1]);
  int res[2]{};
  if (pipe(res) < 0)
  {
    throw std::runtime_error("pipe failed");
  }
  std::unique_ptr< Fd > res_rd = std::make_unique< Fd >(res[0]);
  std::unique_ptr< Fd > res_wr = std::make_unique< Fd >(res[1]);
  std::cout.flush();
  pid_ = fork();
  if (pid_ < 0)
  {
    throw std::runtime_error("fork failed");
  }
  if (pid_ == 0)
  {
    cmd_wr.reset();
    res_rd.reset();
    for (auto i = inherited.cbegin(); i != inherited.cend(); ++i)
    {
      close(*i);
    }
    int code = 0;
    try
    {
      childLoop(cmd_rd->get(), res_wr->get(), seed);
    }
    catch (...)
    {
      code = 1;
    }
    std::_Exit(code);
  }
  to_child_ = std::move(cmd_wr);
  from_child_ = std::move(res_rd);
}

zharov::Worker::~Worker()
{
  const Request stop{RequestKind::STOP, 0, 0, 0, 0};
  sendRequest(to_child_->get(), stop, std::vector< Circle >());
  waitpid(pid_, nullptr, 0);
}

bool zharov::Worker::exchange(const Request& req, const std::vector< Circle >& shapes, Response& res)
{
  return sendRequest(to_child_->get(), req, shapes) && recvResponse(from_child_->get(), res);
}

int zharov::Worker::toChildFd() const noexcept
{
  return to_child_->get();
}

int zharov::Worker::fromChildFd() const noexcept
{
  return from_child_->get();
}
