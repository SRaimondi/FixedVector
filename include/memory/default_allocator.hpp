#pragma once

#include <cstdlib>
#include <new>

namespace FV::Memory {

template<typename T>
class DefaultAllocator
{
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using is_alway_equal = std::true_type;

  template<typename U>
  struct rebind
  {
    using other = DefaultAllocator<U>;
  };

  DefaultAllocator() noexcept = default;

  template<typename U>
  explicit DefaultAllocator(const DefaultAllocator<U>&) noexcept
  {}

  [[nodiscard]] T* allocate(const size_type n) const
  {
    T* ptr{ static_cast<T*>(std::malloc(n * sizeof(T))) };
    if (ptr == nullptr) {
      throw std::bad_alloc();
    }
    return ptr;
  }

  void deallocate(T* ptr, const size_type) const noexcept
  {
    std::free(static_cast<void*>(ptr));
  }
};

template<typename T, typename U>
[[nodiscard]] constexpr bool
operator==(const DefaultAllocator<T>&, const DefaultAllocator<U>&) noexcept
{
  return true;
}

template<typename T, typename U>
[[nodiscard]] constexpr bool
operator!=(const DefaultAllocator<T>&, const DefaultAllocator<U>&) noexcept
{
  return false;
}

} // namespace FV::Memory
