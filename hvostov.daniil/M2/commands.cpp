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
