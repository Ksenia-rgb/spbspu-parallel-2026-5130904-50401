#include "process_db.hpp"

#include <new>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace levkin {
    Pipes::~Pipes() {
        if (in != -1) {
            close(in);
            in = -1;
        }
        if (out != -1) {
            close(out);
            out = -1;
        }
    }

    SharedMemory::SharedMemory(std::string nameParam, size_t size)
        : name(std::move(nameParam)), allocSize(size) {

        fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd == -1) {
            throw std::bad_alloc();
        }

        struct FDGuard {
            int& target_fd;
            bool active = true;
            ~FDGuard() {
                if (active && target_fd != -1) {
                    close(target_fd);
                    target_fd = -1;
                }
            }
        } fdGruard{fd};

        if (ftruncate(fd, allocSize) == -1) {
            throw std::bad_alloc();
        }

        ptr = mmap(nullptr, allocSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED) {
            ptr = nullptr;
            throw std::bad_alloc();
        }

    }

    SharedMemory::~SharedMemory() {
        if (ptr != nullptr && ptr != MAP_FAILED) {
            munmap(ptr, allocSize);
            ptr = nullptr;
        }
    }

    Process::Process(const std::string& procName, const std::string& shmName, size_t shmSize)
        : pipes(), shmem(shmName, shmSize), name(procName) {}

}
