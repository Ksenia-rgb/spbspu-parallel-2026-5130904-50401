#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


namespace levkin {
    struct Pipes   {
        int in = -1;
        int out = -1;

        ~Pipes() {
            if (in != -1) {
                close(in); in = -1;
            }
            if (out != -1) {
                close(out); out = -1;
            }
        }
        Pipes& operator=(const Pipes&) = delete;
        Pipes& operator=(Pipes&&) = delete;
        Pipes(const Pipes&) = delete;
        Pipes(Pipes&&) = delete;
    };
    struct SharedMemory {
        int fd = 0;
        void* ptr = nullptr;
        std::string name;
        size_t allocSize = 4096;
        size_t lstIndex = 0;

        ~SharedMemory(){
            if (ptr != MAP_FAILED && ptr != nullptr) {
                munmap(ptr, allocSize);
            }
        }
        SharedMemory(std::string nameParam) : name(nameParam) {
            fd = shm_open(name.c_str(), O_CREAT  | O_RDWR,  0666);
            if (fd == -1) {
                throw std::bad_alloc();
            }
            if (ftruncate(fd, allocSize) == -1) {
                throw std::bad_alloc();
            }
            ptr = mmap(NULL, allocSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
            if (ptr == MAP_FAILED) {
                throw std::bad_alloc();
            } // todo
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
        ~Process() {
        }
    };
}

#endif
