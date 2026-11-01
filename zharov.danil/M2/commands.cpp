#include "commands.hpp"
#include <iomanip>
#include <ios>
#include <memory>
#include <stdexcept>

namespace
{
  constexpr const char* spaces = " \t\r";

  long long parseNumber(const std::string& word)
  {
    size_t pos = 0;
    const long long value = std::stoll(word, std::addressof(pos));
    if (pos != word.size())
    {
      throw std::invalid_argument("Bad number");
    }
    return value;
  }

  void checkName(const std::string& name)
  {
    const char first = name.front();
    const bool is_letter = (first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z');
    if (!is_letter)
    {
      throw std::invalid_argument("Bad name");
    }
  }

  void printCircle(std::ostream& out, const zharov::Circle& shape)
  {
    out << shape.r << " (" << shape.center.x << " " << shape.center.y << ")\n";
  }

  void printFrame(std::ostream& out, const zharov::Frame& frame)
  {
    out << "(" << frame.left_lower.x << " " << frame.left_lower.y << ") ";
    out << "(" << frame.right_upper.x << " " << frame.right_upper.y << ")\n";
  }

  void printCalc(std::ostream& out, const zharov::Calc& calc)
  {
    out << calc.process << " ";
    if (!calc.done)
    {
      out << "<IN PROGRESS>\n";
      return;
    }
    const std::ios_base::fmtflags flags = out.flags();
    const std::streamsize precision = out.precision();
    out << std::fixed << std::setprecision(3) << calc.ms << " " << calc.area << "\n";
    out.flags(flags);
    out.precision(precision);
  }

  void updateCalc(zharov::Context& ctx, zharov::Calc& calc, zharov::RequestKind kind)
  {
    if (calc.done)
    {
      return;
    }
    zharov::Worker& worker = *ctx.workers.at(calc.process);
    const zharov::Request req{kind, calc.id, 0, 0, 0};
    zharov::Response res{};
    if (!worker.exchange(req, std::vector< zharov::Circle >(), res))
    {
      throw std::runtime_error("Process does not respond");
    }
    if (res.state == zharov::CalcState::DONE)
    {
      calc.done = true;
      calc.ms = res.ms;
      calc.area = res.area;
    }
  }
}

zharov::Args zharov::splitLine(const std::string& line)
{
  Args words;
  size_t start = line.find_first_not_of(spaces);
  while (start != std::string::npos)
  {
    const size_t end = line.find_first_of(spaces, start);
    words.push_back(line.substr(start, end - start));
    start = line.find_first_not_of(spaces, end);
  }
  return words;
}

void zharov::createCircle(std::ostream&, const Args& args, Context& ctx)
{
  constexpr size_t circle_args = 5;
  if (args.size() != circle_args)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  const std::string& name = args[1];
  checkName(name);
  if (ctx.circles.count(name) > 0)
  {
    throw std::logic_error("Circle already exists");
  }
  const long long r = parseNumber(args[2]);
  if (r <= 0)
  {
    throw std::invalid_argument("Bad radius");
  }
  const Point center{parseNumber(args[3]), parseNumber(args[4])};
  ctx.circles.emplace(name, Circle{r, center});
}

void zharov::showShape(std::ostream& out, const Args& args, const Context& ctx)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  printCircle(out, ctx.circles.at(args[1]));
}

void zharov::createSet(std::ostream&, const Args& args, Context& ctx)
{
  constexpr size_t shapes_start = 3;
  if (args.size() < shapes_start)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  const std::string& name = args[1];
  checkName(name);
  if (ctx.sets.count(name) > 0)
  {
    throw std::logic_error("Set already exists");
  }
  const long long size = parseNumber(args[2]);
  if (size <= 0 || static_cast< size_t >(size) != args.size() - shapes_start)
  {
    throw std::invalid_argument("Bad set size");
  }
  std::vector< Circle > shapes;
  for (size_t i = shapes_start; i < args.size(); ++i)
  {
    shapes.push_back(ctx.circles.at(args[i]));
  }
  ctx.sets.emplace(name, shapes);
}

void zharov::showSet(std::ostream& out, const Args& args, const Context& ctx)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  const std::vector< Circle >& shapes = ctx.sets.at(args[1]);
  for (auto i = shapes.cbegin(); i != shapes.cend(); ++i)
  {
    printCircle(out, *i);
  }
}

void zharov::showFrame(std::ostream& out, const Args& args, const Context& ctx)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  printFrame(out, getFrame(ctx.circles.at(args[1])));
}

void zharov::showFrameSet(std::ostream& out, const Args& args, const Context& ctx)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  printFrame(out, getFrame(ctx.sets.at(args[1])));
}

void zharov::spawnProcess(std::ostream&, const Args& args, Context& ctx)
{
  if (args.size() != 3)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  const std::string& name = args[1];
  checkName(name);
  if (ctx.workers.count(name) > 0)
  {
    throw std::logic_error("Process already exists");
  }
  const long long seed = parseNumber(args[2]);
  if (seed < 0)
  {
    throw std::invalid_argument("Bad seed");
  }
  std::vector< int > inherited;
  for (auto i = ctx.workers.cbegin(); i != ctx.workers.cend(); ++i)
  {
    inherited.push_back(i->second->toChildFd());
    inherited.push_back(i->second->fromChildFd());
  }
  ctx.workers.emplace(name, std::make_unique< Worker >(static_cast< size_t >(seed), inherited));
}

void zharov::startArea(std::ostream&, const Args& args, Context& ctx)
{
  constexpr size_t area_args = 6;
  if (args.size() != area_args)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  const std::string& process = args[1];
  const std::string& name = args[2];
  checkName(name);
  if (ctx.calcs.count(name) > 0)
  {
    throw std::logic_error("Calculation already exists");
  }
  Worker& worker = *ctx.workers.at(process);
  const std::vector< Circle >& shapes = ctx.sets.at(args[3]);
  const long long threads = parseNumber(args[4]);
  const long long tries = parseNumber(args[5]);
  if (threads <= 0 || tries <= 0)
  {
    throw std::invalid_argument("Bad threads or tries");
  }

  const size_t id = ctx.calcs.size();
  const Request req{RequestKind::CALC, id, static_cast< size_t >(threads), static_cast< size_t >(tries), shapes.size()};
  Response res{};
  if (!worker.exchange(req, shapes, res))
  {
    throw std::runtime_error("Process does not respond");
  }
  ctx.calcs.emplace(name, Calc{process, id, false, 0.0, 0.0});
}

void zharov::showStatus(std::ostream& out, const Args& args, Context& ctx)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  Calc& calc = ctx.calcs.at(args[1]);
  updateCalc(ctx, calc, RequestKind::STATUS);
  printCalc(out, calc);
}

void zharov::waitCalc(std::ostream& out, const Args& args, Context& ctx)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("Bad arguments count");
  }
  Calc& calc = ctx.calcs.at(args[1]);
  updateCalc(ctx, calc, RequestKind::WAIT);
  printCalc(out, calc);
}
