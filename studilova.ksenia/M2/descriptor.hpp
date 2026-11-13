#ifndef DESCRIPTOR_HPP
#define DESCRIPTOR_HPP

namespace studilova
{
  class Descriptor
  {
  public:
    Descriptor() noexcept;
    explicit Descriptor(int descriptor) noexcept;
    Descriptor(const Descriptor&) = delete;
    Descriptor(Descriptor&& other) noexcept;
    ~Descriptor();

    Descriptor& operator=(const Descriptor&) = delete;
    Descriptor& operator=(Descriptor&& other) noexcept;

    int get() const noexcept;
    int release() noexcept;

  private:
    int descriptor_;
  };
}

#endif
