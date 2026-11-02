#include "commands.hpp"
#include <stdexcept>
#include <limits>
#include <sys/wait.h>
#include <cerrno>
#include <system_error>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <cstdio>
#include <csignal>
#include <algorithm>
#include "calculation.hpp"
#include "ipc.hpp"
#include "FdGuard.hpp"
#include "child.hpp"

namespace {
  bool tryReadResult(hvostov::info_t& info, hvostov::process_t& p)
  {
    struct pollfd pfd = {p.from_child, POLLIN, 0};
    if (poll(&pfd, 1, 0) <= 0) {
      return false;
    }

    char sig = 0;
    int err = 0;
    const size_t n = hvostov::recv(err, p.from_child, &sig, 1);
    if (n != 1) {
      return false;
    }

    hvostov::ShmLayout* layout = reinterpret_cast< hvostov::ShmLayout* >(p.shm->get());
    const std::string calc_name(layout->res.calc_name, strnlen(layout->res.calc_name, sizeof(layout->res.calc_name)));

    auto it = info.calc.find(calc_name);
    if (it != info.calc.end()) {
      it->second.done = true;
      it->second.result = layout->res.result;
      it->second.time_ms = layout->res.time_ms;
    }

    auto pit = std::find(p.pending.begin(), p.pending.end(), calc_name);
    if (pit != p.pending.end()) {
      p.pending.erase(pit);
    }
    return true;
  }

}

void hvostov::addCircle(std::istream& in, std::ostream&, info_t& info)
{
  std::string name;
  double r = 0.0, x = 0.0, y = 0.0;
  in >> name >> r >> x >> y;

  if (!in || r <= 0.0) {
    throw std::logic_error("bad circle params");
  }
  if (info.db.count(name) || info.set.count(name)) {
    throw std::logic_error("name already exists");
  }

  shape_t s{};
  s.rx = r;
  s.ry = 0.0;
  s.x = x;
  s.y = y;
  s.angle = 0.0;
  s.is_ellipse = false;
  info.db[name] = s;
}

void hvostov::addEllipse(std::istream& in, std::ostream&, info_t& info)
{
  std::string name;
  double rx = 0.0, ry = 0.0, x = 0.0, y = 0.0;
  in >> name >> rx >> ry >> x >> y;

  if (!in || rx <= 0.0 || ry <= 0.0) {
    throw std::logic_error("bad ellipse params");
  }
  if (info.db.count(name) || info.set.count(name)) {
    throw std::logic_error("name already exists");
  }

  shape_t s{};
  s.rx = rx;
  s.ry = ry;
  s.x = x;
  s.y = y;
  s.angle = 0.0;
  s.is_ellipse = true;
  info.db[name] = s;
}

void hvostov::rotate(std::istream& in, std::ostream&, info_t& info)
{
  std::string name;
  double ksi = 0.0;
  in >> name >> ksi;

  if (!in) {
    throw std::logic_error("bad rotate args");
  }

  auto it = info.db.find(name);
  if (it == info.db.end()) {
    throw std::logic_error("shape not found");
  }
  it->second.angle += ksi;
}

void hvostov::show(std::istream& in, std::ostream& out, info_t& info)
{
  std::string name;
  in >> name;
  if (!in || info.db.find(name) == info.db.end()) {
    throw std::logic_error("shape not found");
  }
  out << info.db[name] << "\n";
}

void hvostov::set(std::istream& in, std::ostream&, info_t& info)
{
  std::string name;
  size_t size = 0;
  in >> name >> size;

  if (!in || size == 0) {
    throw std::logic_error("bad set size");
  }
  if (info.set.count(name) || info.db.count(name)) {
    throw std::logic_error("name already exists");
  }

  std::vector< shape_t > v;
  v.reserve(size);

  for (size_t i = 0; i < size; ++i) {
    std::string sname;
    in >> sname;
    if (!in) {
      throw std::logic_error("not enough shapes");
    }
    auto it = info.db.find(sname);
    if (it == info.db.end()) {
      throw std::logic_error("shape not found");
    }
    v.push_back(it->second);
  }

  in.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
  info.set[name] = std::move(v);
}

void hvostov::showSet(std::istream& in, std::ostream& out, info_t& info)
{
  std::string name;
  in >> name;
  if (!in || info.set.find(name) == info.set.end()) {
    throw std::logic_error("set not found");
  }
  for (const shape_t& s : info.set[name]) {
    out << s << "\n";
  }
}

void hvostov::getFrame(std::istream& in, std::ostream& out, info_t& info)
{
  std::string name;
  in >> name;
  if (!in || info.db.find(name) == info.db.end()) {
    throw std::logic_error("shape not found");
  }
  out << calculateFrame(info.db[name]) << "\n";
}

void hvostov::getFrameSet(std::istream& in, std::ostream& out, info_t& info)
{
  std::string name;
  in >> name;
  if (!in || info.set.find(name) == info.set.end()) {
    throw std::logic_error("set not found");
  }
  out << calculateFrame(info.set[name]) << "\n";
}

void hvostov::spawnProcess(std::istream& in, std::ostream&, info_t& info)
{
  std::string name;
  long long seed_ll = 0;
  in >> name >> seed_ll;

  if (!in || seed_ll < 0) {
    throw std::logic_error("bad spawn args");
  }
  if (info.procs.count(name)) {
    throw std::logic_error("process already exists");
  }

  const size_t seed = static_cast< size_t >(seed_ll);
  const std::string shm_name = "/lab2_" + name + "_" + std::to_string(getpid());

  auto shm = std::make_shared< ShmGuard >(shm_name, sizeof(ShmLayout), true);

  int to_child[2] = {-1, -1};
  int from_child[2] = {-1, -1};
  if (pipe(to_child) != 0 || pipe(from_child) != 0) {
    throw std::system_error(errno, std::generic_category(), "pipe");
  }

  FdGuard tc_r(to_child[0]);
  FdGuard tc_w(to_child[1]);
  FdGuard fc_r(from_child[0]);
  FdGuard fc_w(from_child[1]);

  const pid_t pid = fork();
  if (pid < 0) {
    throw std::system_error(errno, std::generic_category(), "fork");
  }

  if (pid == 0) {
    tc_w.close();
    fc_r.close();
    const int rc = runChild(seed, tc_r.get(), fc_w.get(), reinterpret_cast< ShmLayout* >(shm->get()));
    _exit(rc);
  }

  tc_r.close();
  fc_w.close();

  process_t p{};
  p.pid = pid;
  p.to_child = tc_w.release();
  p.from_child = fc_r.release();
  p.shm = std::move(shm);
  p.seed = seed;
  info.procs[name] = std::move(p);
}

void hvostov::areaOn(std::istream& in, std::ostream&, info_t& info)
{
  std::string proc_name, calc_name, set_name;
  long long threads_ll = 0;
  size_t tries = 0;

  in >> proc_name >> calc_name >> set_name >> threads_ll >> tries;

  if (!in || threads_ll <= 0 || tries == 0) {
    throw std::logic_error("bad area-on args");
  }
  if (info.procs.find(proc_name) == info.procs.end()) {
    throw std::logic_error("process not found");
  }
  if (info.calc.count(calc_name)) {
    throw std::logic_error("calc already exists");
  }
  if (info.set.find(set_name) == info.set.end()) {
    throw std::logic_error("set not found");
  }

  process_t& p = info.procs[proc_name];
  if (!p.pending.empty()) {
    throw std::logic_error("process is busy");
  }

  const std::vector< shape_t >& shapes = info.set[set_name];
  if (shapes.empty() || shapes.size() > MAX_SHAPES) {
    throw std::logic_error("bad shape count");
  }

  ShmLayout* layout = reinterpret_cast< ShmLayout* >(p.shm->get());

  std::snprintf(layout->req.calc_name, sizeof(layout->req.calc_name), "%s", calc_name.c_str());
  layout->req.threads = static_cast< int >(threads_ll);
  layout->req.tries = tries;
  layout->req.shape_count = shapes.size();
  layout->req.seed = p.seed;
  std::copy(shapes.begin(), shapes.end(), layout->shapes);

  int err = 0;
  const char sig = 1;
  if (send(err, p.to_child, &sig, 1) != 1) {
    throw std::system_error(errno, std::generic_category(), "send signal");
  }

  p.pending.push_back(calc_name);
  info.calc[calc_name] = {proc_name, false, false, 0.0, 0.0};
}

void hvostov::status(std::istream& in, std::ostream& out, info_t& info)
{
  std::string calc_name;
  in >> calc_name;

  if (!in || info.calc.find(calc_name) == info.calc.end()) {
    throw std::logic_error("calc not found");
  }

  calc_state_t& st = info.calc[calc_name];
  if (!st.done && !st.canceled) {
    auto it = info.procs.find(st.process_name);
    if (it != info.procs.end()) {
      tryReadResult(info, it->second);
    }
  }
  out << st << "\n";
}

void hvostov::wait(std::istream& in, std::ostream& out, info_t& info)
{
  std::string calc_name;
  in >> calc_name;

  if (!in || info.calc.find(calc_name) == info.calc.end()) {
    throw std::logic_error("calc not found");
  }

  calc_state_t& st = info.calc[calc_name];
  if (st.canceled) {
    out << st << "\n";
    return;
  }

  if (!st.done) {
    auto it = info.procs.find(st.process_name);
    if (it == info.procs.end()) {
      throw std::logic_error("process not found");
    }

    process_t& p = it->second;
    char sig = 0;
    int err = 0;
    const size_t n = recv(err, p.from_child, &sig, 1);
    if (n != 1) {
      throw std::runtime_error("failed to read done signal");
    }

    ShmLayout* layout = reinterpret_cast< ShmLayout* >(p.shm->get());
    st.done = true;
    st.result = layout->res.result;
    st.time_ms = layout->res.time_ms;

    auto pit = std::find(p.pending.begin(), p.pending.end(), calc_name);
    if (pit != p.pending.end()) {
      p.pending.erase(pit);
    }
  }
  out << st << "\n";
}

void hvostov::timeout(std::istream& in, std::ostream& out, info_t& info)
{
  std::string calc_name;
  int ms = 0;
  in >> calc_name >> ms;

  if (!in || ms < 0 || info.calc.find(calc_name) == info.calc.end()) {
    throw std::logic_error("bad timeout args");
  }

  calc_state_t& st = info.calc[calc_name];
  if (st.canceled) {
    out << st << "\n";
    return;
  }

  if (!st.done) {
    auto it = info.procs.find(st.process_name);
    if (it != info.procs.end()) {
      process_t& p = it->second;
      struct pollfd pfd = {p.from_child, POLLIN, 0};
      if (poll(&pfd, 1, ms) > 0) {
        char sig = 0;
        int err = 0;
        if (recv(err, p.from_child, &sig, 1) == 1) {
          ShmLayout* layout = reinterpret_cast< ShmLayout* >(p.shm->get());
          st.done = true;
          st.result = layout->res.result;
          st.time_ms = layout->res.time_ms;

          auto pit = std::find(p.pending.begin(), p.pending.end(), calc_name);
          if (pit != p.pending.end()) {
            p.pending.erase(pit);
          }
        }
      }
    }
  }
  out << st << "\n";
}

void hvostov::killProcess(std::istream& in, std::ostream&, info_t& info)
{
  std::string proc_name;
  in >> proc_name;

  if (!in || info.procs.find(proc_name) == info.procs.end()) {
    throw std::logic_error("process not found");
  }

  process_t& p = info.procs[proc_name];

  for (const std::string& calc_name : p.pending) {
    auto it = info.calc.find(calc_name);
    if (it != info.calc.end()) {
      it->second.canceled = true;
    }
  }
  p.pending.clear();

  if (p.pid > 0) {
    ::kill(p.pid, SIGTERM);
    int status = 0;
    waitpid(p.pid, &status, 0);
    p.pid = -1;
  }

  if (p.to_child >= 0) {
    close(p.to_child);
    p.to_child = -1;
  }
  if (p.from_child >= 0) {
    close(p.from_child);
    p.from_child = -1;
  }

  info.procs.erase(proc_name);
}
