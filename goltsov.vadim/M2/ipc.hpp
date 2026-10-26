#ifndef IPC_HPP
#define IPC_HPP

#include <cstddef>
#include <stdexcept>

namespace goltsov
{
  size_t send(int& err, int wr, const char* b, size_t k);
  size_t recv(int& err, int rd, char* b, size_t k);

  template< class T >
  void send_value(int wr, const T& value)
  {
    int err = 0;
    if (send(err, wr, reinterpret_cast< const char* >(&value), sizeof(T)) != sizeof(T))
    {
      throw std::runtime_error("send failed");
    }
  }

  template< class T >
  void recv_value(int rd, T& value)
  {
    int err = 0;
    if (recv(err, rd, reinterpret_cast< char* >(&value), sizeof(T)) != sizeof(T))
    {
      throw std::runtime_error("recv failed");
    }
  }
}

#endif
