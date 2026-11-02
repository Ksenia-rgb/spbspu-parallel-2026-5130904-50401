#include "shared_memory.hpp"
#include <stdexcept>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace tarasenko {
  Ellipse* allocateSharedCircles(const char* name, size_t count)
  {
    const int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
      throw std::runtime_error("Cannot create shared memory");
    }
    const size_t bytes = count * sizeof(Ellipse);
    if (ftruncate(fd, bytes) == -1) {
      close(fd);
      shm_unlink(name);
      throw std::runtime_error("Cannot resize shared memory");
    }
    void* memory = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (memory == MAP_FAILED) {
      shm_unlink(name);
      throw std::runtime_error("Cannot map shared memory");
    }
    return static_cast< Ellipse* >(memory);
  }

  void freeSharedCircles(const char* name, Ellipse* ellipses, size_t count)
  {
    munmap(ellipses, count * sizeof(Ellipse));
    shm_unlink(name);
  }
}
