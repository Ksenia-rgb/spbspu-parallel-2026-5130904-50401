#include <iostream>
#include <string>
#include <csignal>
#include <unistd.h>
#include <sys/wait.h>

#include "command.hpp"
#include "parse.hpp"

int main()
{
  struct sigaction sa{};
  sa.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sa, nullptr);
  sedov::CommandContext ctx;
  const sedov::CommandTable table = sedov::makeCommandTable();
  std::string line;
  while (std::getline(std::cin, line)) {
    const std::vector< std::string > words = sedov::split(line);
    if (words.empty()) {
      continue;
    }
    auto it = table.find(words[0]);
    if (it == table.end()) {
      std::cout << "<UNKNOWN COMMAND>\n";
      continue;
    }
    it->second(words, ctx, std::cout);
  }
  for (sedov::ProcessMap::iterator it = ctx.processes.begin(); it != ctx.processes.end(); ++it) {
    sedov::Process & proc = it->second;
    if (proc.alive) {
      close(proc.in_fd);
      close(proc.out_fd);
      waitpid(proc.pid, nullptr, 0); 
    }
  }
  return 0;
}
