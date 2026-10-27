#include "process_db.hpp"

#include <new>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace levkin {
  Pipes::~Pipes()
  {
    if (in != -1) {
      close(in);
      in = -1;
    }
    if (out != -1) {
      close(out);
      out = -1;
    }
  }

  SharedMemory::SharedMemory(size_t size):
    allocSize(size), lstIndex(0)
  {
    ptr = mmap(nullptr, allocSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
      ptr = nullptr;
      throw std::bad_alloc();
    }
  }

  SharedMemory::~SharedMemory()
  {
    if (ptr != nullptr && ptr != MAP_FAILED) {
      munmap(ptr, allocSize);
      ptr = nullptr;
    }
  }

  Process::Process(const std::string& procName, size_t shmSize):
    pipes(),
    shmem(shmSize),
    name(procName)
  {}

}
