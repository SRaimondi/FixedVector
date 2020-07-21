#pragma once

#include "memory.hpp"

#include <cstddef>
#include <cstdlib>
#include <new>

namespace FV::Memory {

template<typename T>
class DefaultAllocator
{
private:
  static constexpr bool is_overaligned{ std::alignment_of_v<T> >
                                        std::alignment_of_v<std::max_align_t> };

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

  [[nodiscard]] value_type* allocate(const size_type n) const
  {
    value_type* ptr{ [n]() -> value_type* {
      if constexpr (is_overaligned) {
        return static_cast<T*>(
          allocateAligned<value_type>(std::alignment_of_v<value_type>, n));
      } else {
        return static_cast<T*>(std::malloc(n * sizeof(T)));
      }
    }() };

    if (ptr == nullptr) {
      throw std::bad_alloc{};
    }

    return ptr;
  }

  void deallocate(value_type* ptr, const size_type) const noexcept
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
