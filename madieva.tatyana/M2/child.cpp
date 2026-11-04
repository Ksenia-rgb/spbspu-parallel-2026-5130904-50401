#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include "file_descriptor.hpp"
#include "message.hpp"
#include "parser.hpp"
#include "circle.hpp"

namespace
{

}

int main(int argc, char ** argv)
{
  if (argc != 4)
  {
    std::cerr << "usage: child <cmd_fd> <res_fd> <seed>\n";
    return 1;
  }

  const std::string cmd_fd_s = argv[1];
  const std::string res_fd_s = argv[2];
  const std::string seed_s = argv[3];

  try
  {
    const int cmd_fd = static_cast< int >(madieva::parseInteger(cmd_fd_s));
    const int res_fd = static_cast< int >(madieva::parseInteger(res_fd_s));
    const long long seed = madieva::parseInteger(seed_s);

    madieva::FileDescriptor cmd_rd(cmd_fd);
    madieva::FileDescriptor res_wr(res_fd);

    std::size_t size = 0;
    madieva::recvAll(cmd_rd.get(), reinterpret_cast< char * >(&size), sizeof(size));

    std::string msg(size, '\0');
    madieva::recvAll(cmd_rd.get(), msg.data(), msg.size());

    const madieva::Args args = madieva::split(msg);
    const madieva::AreaRequest req = madieva::parseAreaRequest(args);

    return 0;
  }
  catch (const std::exception & e)
  {
    std::cerr << "child error: " << e.what() << "\n";
    return 2;
  }
}