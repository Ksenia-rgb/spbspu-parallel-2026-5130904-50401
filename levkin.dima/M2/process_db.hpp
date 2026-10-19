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
    };
    struct SharedMemory {
        int fd = 0;
        void* ptr = nullptr;
        const char * name;
        size_t allocSize = 4096;
        size_t lstIndex = 0;

        ~SharedMemory(){
            if (ptr != MAP_FAILED && ptr != nullptr) {
                munmap(ptr, allocSize);
            }
        }
        SharedMemory(const char* nameParam) : name(nameParam) {
            fd = shm_open(name, O_CREAT  | O_RDWR,  0666);
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
