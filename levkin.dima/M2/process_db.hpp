#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <cstddef>
#include <vector>

namespace levkin {

    struct Pipes {
        int in = -1;
        int out = -1;

        Pipes() = default;
        ~Pipes();

        Pipes& operator=(const Pipes&) = delete;
        Pipes& operator=(Pipes&&) = delete;
        Pipes(const Pipes&) = delete;
        Pipes(Pipes&&) = delete;
    };

    struct SharedMemory {
        void* ptr = nullptr;
        size_t allocSize = 4096;
        size_t lstIndex = 0;

        SharedMemory(size_t size = 4096);
        ~SharedMemory();

        SharedMemory& operator=(const SharedMemory&) = delete;
        SharedMemory& operator=(SharedMemory&&) = delete;
        SharedMemory(const SharedMemory&) = delete;
        SharedMemory(SharedMemory&&) = delete;

    private:
        int fd = -1;
    };

    struct Process {
        Pipes pipes;
        SharedMemory shmem;
        std::string name;

        Process(const std::string& procName, size_t shmSize = 4096);
        ~Process() = default;
    };
    struct ProcessDB {
      std::vector<Process> processes  ;
    };

}

#endif
