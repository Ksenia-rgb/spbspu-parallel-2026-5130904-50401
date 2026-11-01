#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <sys/types.h>

namespace kondrat
{
  class Process
  {
  public:
    Process(long long seed, const std::vector< int > & inherited_descriptors);
    Process(const Process &) = delete;
    Process & operator=(const Process &) = delete;
    ~Process();

    int requestDescriptor() const;
    int resultDescriptor() const;

  private:
    pid_t pid_;
    int request_descriptor_;
    int result_descriptor_;
  };

  using ProcessMap = std::map< std::string, std::unique_ptr< Process > >;

  void spawnProcess(const std::vector< std::string > & words, ProcessMap & processes);
}

#endif
