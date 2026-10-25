#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string>
#include <map>
#include <vector>
#include <limits>
#include "shared_circles.hpp"

namespace goltsov
{
  const char* const SHMN = "shm-temp";

  struct CirclePtr
  {
    std::string name;
    size_t idx;
  };

  struct Process
  {
    std::string name;
    int wr; //в родителе пишем сюда
    int rd; //в родителе читаем отчюда
    size_t seed;
    pid_t pid;
  };

  std::map< std::string, size_t > name_idx_map;
  std::map< std::string, std::vector< std::string > > sets;
  std::map< std::string, Process > processes;
  std::map< std::string, Process > statuses;

  size_t send(int& err, int wr, const char* b, size_t k) {
    size_t r = 0;
    while (r < k) {
      err = write(wr, b + r, k - r);
      if (err < 0) break;
      r += err;
    }
    return r;
  }

  size_t recv(int& err, int rd, char* b, size_t k) {
    size_t r = 0;
    while (r < k) {
      err = read(rd, b + r, k - r);
      if (err <= 0) break;
      r += err;
    }
    return r;
  }

  void circle(goltsov::SharedCircles& shared_circles)
  {
    std::string name;
    double r;
    double x, y;
    std::cin >> name >> r >> x >> y;
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    shared_circles.new_circle(r, x, y);
    name_idx_map.insert({name, shared_circles.get_count_circles()});
  }

  void set(goltsov::SharedCircles& shared_circles)
  {
    std::string name;
    size_t set_size;
    std::vector< std::string > shapes;
    std::cin >> name >> set_size;
    for (size_t i = 0; i < set_size; ++i)
    {
      std::string shapei;
      std::cin >> shapei;
      shapes.push_back(shapei);
    }
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    sets[name] = shapes;
  }

  void show(goltsov::SharedCircles& shared_circles)
  {
    std::string shape_name;
    std::cin >> shape_name;
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    Circle* circle = shared_circles.get_data() + name_idx_map.at(shape_name);
    std::cout << *circle << "\n";
  }

  void showset(goltsov::SharedCircles& shared_circles)
  {
    std::string set_name;
    std::cin >> set_name;
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    std::vector< std::string > set = sets.at(set_name);
    for (size_t i = 0; i < set.size(); ++i)
    {
      std::cout << name_idx_map.at(set[i]) << "\n";
    }
  }

  void frame(goltsov::SharedCircles& shared_circles)
  {
    std::string shape_name;
    std::cin >> shape_name;
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    Circle* circle = shared_circles.get_data() + name_idx_map.at(shape_name);
    std::cout << "(" << circle->x - circle->r << " " << circle->y - circle->r << ") (" << circle->x + circle->r << " " << circle->y + circle->r << ")" << "\n";
  }

  void frameset(goltsov::SharedCircles& shared_circles)
  {
    std::string set_name;
    std::cin >> set_name;
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    std::vector< std::string > set = sets.at(set_name);
    double min_x = std::numeric_limits< double >::max();
    double max_x = std::numeric_limits< double >::min();
    double min_y = std::numeric_limits< double >::max();
    double max_y = std::numeric_limits< double >::min();
    for (size_t i = 0; i < set.size(); ++i)
    {
      Circle* circle = shared_circles.get_data() + name_idx_map.at(set[i]);
      min_x = std::min< double >(min_x, circle->x - circle->r);
      max_x = std::max< double >(max_x, circle->x + circle->r);
      min_y = std::min< double >(min_y, circle->y - circle->r);
      max_y = std::max< double >(max_y, circle->y + circle->r);
    }
    std::cout << "(" << min_x << " " << min_y << ") (" << max_x << " " << max_y << ")" << "\n";
  }

  void spawn(goltsov::SharedCircles& shared_circles)
  {
    std::string process_name;
    size_t seed;
    std::cin >> process_name >> seed;
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    int pps1[2];
    pipe(pps1);
    int pps2[2];
    pipe(pps2);
    pid_t pid = fork();
    if (pid < 0)
    {
      throw std::runtime_error("pid");
    }
    if (pid == 0)
    {
      execl("child", process_name.c_str(), pps1[0], pps2[1], seed, NULL);
      _exit(1);
    }
    Process process = Process{process_name, pps1[1], pps2[0], seed, pid};
    processes.insert({process_name, std::move(process)});
  }

  void areas_on(goltsov::SharedCircles& shared_circles)
  {
    std::string process_name, calc_name, set_name;
    size_t threads, tries;
    std::cin >> process_name >> calc_name >> set_name >> threads >> tries;
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    std::vector< std::string > set = sets.at(set_name);
    Process process = processes.at(process_name);
    int err = 0;
    size_t count_circles = shared_circles.get_count_circles();
    size_t count_circles_in_set = set.size();
    send(err, process.wr, (char*)&(count_circles), sizeof(count_circles));
    send(err, process.wr, (char*)&(threads), sizeof(threads));
    send(err, process.wr, (char*)&(tries), sizeof(tries));
    send(err, process.wr, (char*)&(count_circles_in_set), sizeof(count_circles_in_set));
    for (size_t i = 0; i < count_circles_in_set; ++i)
    {
      size_t idxi = name_idx_map.at(set[i]);
      send(err, process.wr, (char*)&(idxi), sizeof(idxi));
    }
    statuses.insert({calc_name, std::move(process)});
  }

  void status(goltsov::SharedCircles& shared_circles)
  {
    std::string calc_name;
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    Process process = statuses.at(calc_name);
    int status = 0;
    pid_t r = waitpid(process.pid, &status, WNOHANG);
    if (r == 0)
    {
      std::cout << process.name << " <IN PROGRESS>\n";
    }
    else if (r == process.pid)
    {
      int err = 0;
      size_t ms;
      recv(err, process.rd, (char*)&ms, sizeof(ms));
      double res;
      recv(err, process.rd, (char*)&res, sizeof(res));
      printf("%s %zu %.3f\n",process.name, ms, res);
    }
    else
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
  }

  void wait(goltsov::SharedCircles& shared_circles)
  {
    std::string calc_name;
    if (!std::cin)
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    Process process = statuses.at(calc_name);
    int status = 0;
    pid_t r = waitpid(process.pid, &status, 0);

    if (r == process.pid)
    {
      int err = 0;
      size_t ms;
      recv(err, process.rd, (char*)&ms, sizeof(ms));
      double res;
      recv(err, process.rd, (char*)&res, sizeof(res));
      printf("%s %zu %.3f\n",process.name, ms, res);
    }
    else
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
  }
}

int main()
{
  goltsov::SharedCircles shared_circles = goltsov::SharedCircles(goltsov::SHMN, 10);

}
