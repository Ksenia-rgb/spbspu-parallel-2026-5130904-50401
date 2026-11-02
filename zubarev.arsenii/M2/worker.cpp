#include "worker.hpp"
#include <chrono>
#include <cstring>
#include <vector>
#include "ipc.hpp"
#include "monte_carlo.hpp"
#include "shapes.hpp"

void zubarev::run_worker(int req_fd, int resp_fd, size_t seed)
{
  while (true) {
    RequestHeader req{};
    int err = 0;
    size_t rec = recv(err, req_fd, reinterpret_cast< char* >(&req), sizeof(req));

    if (rec == 0 && err == 0) {
      break;
    }
    if (rec < sizeof(req) || err < 0) {
      break;
    }

    std::vector< Circle > circles(req.count);
    if (req.count > 0) {
      size_t circles_bytes = req.count * sizeof(Circle);
      rec = recv(err, req_fd, reinterpret_cast< char* >(circles.data()), circles_bytes);
      if (rec < circles_bytes || err < 0) {
        break;
      }
    }

    auto start = std::chrono::steady_clock::now();
    double calculated_area = area(circles, req.threads, req.tries, seed);
    auto finish = std::chrono::steady_clock::now();

    std::chrono::duration< double, std::milli > elapsed = finish - start;

    Response resp{};
    std::strncpy(resp.name, req.name, sizeof(resp.name) - 1);
    resp.name[sizeof(resp.name) - 1] = '\0';
    resp.ms = elapsed.count();
    resp.area = calculated_area;

    size_t sent = send(err, resp_fd, reinterpret_cast< const char* >(&resp), sizeof(resp));
    if (sent < sizeof(resp) || err < 0) {
      break;
    }
  }
}
