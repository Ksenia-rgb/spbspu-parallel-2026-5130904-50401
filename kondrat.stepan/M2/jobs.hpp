#ifndef JOBS_HPP
#define JOBS_HPP

#include <deque>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "process.hpp"
#include "shape.hpp"

namespace kondrat
{
  enum class JobState
  {
    InProgress,
    Done,
    Canceled
  };

  struct Job
  {
    unsigned long long id;
    std::string process_name;
    long long threads;
    long long tries;
    std::vector< Shape > shapes;
    bool started;
    JobState state;
    double time_ms;
    double area;
  };

  class Jobs
  {
  public:
    explicit Jobs(ProcessMap & processes);

    void submit(
      const std::vector< std::string > & words,
      const std::map< std::string, Shape > & shapes,
      const std::map< std::string, std::vector< std::string > > & sets);
    void show(const std::vector< std::string > & words, bool wait, std::ostream & output);
    void collectReady();
    void finishAll();
    std::vector< int > resultDescriptors() const;

  private:
    ProcessMap & processes_;
    std::map< std::string, Job > jobs_;
    std::map< std::string, std::deque< std::string > > queues_;
    std::set< std::string > failed_processes_;
    unsigned long long next_id_;

    void startNext(const std::string & process_name);
    void receive(const std::string & process_name);
    void cancelProcess(const std::string & process_name);
  };
}

#endif
