#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <new>
#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace levkin {

    struct Pipes {
        int in = -1;
        int out = -1;

        Pipes() = default;

        ~Pipes() {
            if (in != -1) {
                close(in);
                in = -1;
            }
            if (out != -1) {
                close(out);
                out = -1;
            }
        }

        Pipes& operator=(const Pipes&) = delete;
        Pipes& operator=(Pipes&&) = delete;
        Pipes(const Pipes&) = delete;
        Pipes(Pipes&&) = delete;
    };

    struct SharedMemory {
        int fd = -1;
        void* ptr = MAP_FAILED;
        std::string name;
        size_t allocSize = 4096;
        size_t lstIndex = 0;

        ~SharedMemory() {
            if (ptr != MAP_FAILED && ptr != nullptr) {
                munmap(ptr, allocSize);
                ptr = MAP_FAILED;
            }
            if (fd != -1) {
                close(fd);
                fd = -1;
            }
        }

        SharedMemory(std::string nameParam, size_t size = 4096)
            : name(std::move(nameParam)), allocSize(size) {

            fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
            struct RAIIFD {
                int& fd_ref;

                explicit RAIIFD(int& f) : fd_ref(f) {}
                ~RAIIFD() {
                    if (fd_ref != -1) {
                        close(fd_ref);
                        fd_ref = -1;
                    }
                }
            } guard{fd};


            if (ftruncate(fd, allocSize) == -1) {
                throw std::bad_alloc();
            }

            ptr = mmap(nullptr, allocSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (ptr == MAP_FAILED) {
                throw std::bad_alloc();
            }
        }

        SharedMemory& operator=(const SharedMemory&) = delete;
        SharedMemory& operator=(SharedMemory&&) = delete;
        SharedMemory(const SharedMemory&) = delete;
        SharedMemory(SharedMemory&&) = delete;
    };

    struct Process {
        Pipes pipes;
        SharedMemory shmem;
        std::string name;

        Process(const std::string& procName, const std::string& shmName, size_t shmSize = 4096)
            : pipes(), shmem(shmName, shmSize), name(procName) {}

        ~Process() = default;
    };

}

#endif
