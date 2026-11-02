#ifndef IPC_HPP
#define IPC_HPP

#include <cstddef>
#include <cstdint>

namespace zubarev {
  struct RequestHeader {
    char name[32];
    uint64_t threads;
    uint64_t tries;
    uint64_t count;
  };

  struct Response {
    char name[32];
    double ms;
    double area;
  };

  size_t send(int& err, int wr, const char* b, size_t k);
  size_t recv(int& err, int rd, char* b, size_t k);

  class Pipe {
  public:
    Pipe();
    ~Pipe();

    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;

    Pipe(Pipe&& other) noexcept;
    Pipe& operator=(Pipe&& other) noexcept;

    int readFd() const noexcept;
    int writeFd() const noexcept;
    int getReadFd() const noexcept;
    int gerReadFd() const noexcept;

    int releaseRead() noexcept;
    int releaseWrite() noexcept;

    void closeRead() noexcept;
    void closeWrite() noexcept;
    void closeAll() noexcept;

  private:
    int fds_[2];
  };
}

#endif
