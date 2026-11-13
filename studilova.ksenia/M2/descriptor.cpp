#include "descriptor.hpp"
#include <unistd.h>

studilova::Descriptor::Descriptor() noexcept:
  descriptor_(-1)
{}

studilova::Descriptor::Descriptor(int descriptor) noexcept:
  descriptor_(descriptor)
{}

studilova::Descriptor::Descriptor(Descriptor&& other) noexcept:
  descriptor_(other.release())
{}

studilova::Descriptor::~Descriptor()
{
  if (descriptor_ != -1)
  {
    close(descriptor_);
  }
}

studilova::Descriptor& studilova::Descriptor::operator=(Descriptor&& other) noexcept
{
  if (this != &other)
  {
    if (descriptor_ != -1)
    {
      close(descriptor_);
    }
    descriptor_ = other.release();
  }
  return *this;
}

int studilova::Descriptor::get() const noexcept
{
  return descriptor_;
}

int studilova::Descriptor::release() noexcept
{
  const int descriptor = descriptor_;
  descriptor_ = -1;
  return descriptor;
}
