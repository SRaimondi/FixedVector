#pragma once

#include "memory.hpp"

#include <type_traits>
#include <new>

namespace FV::Memory {

template<typename T, std::size_t alignment = std::alignment_of_v<T>>
class AlignedAllocator
{
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using is_alway_equal = std::true_type;

  template<typename U>
  struct rebind
  {
    using other = AlignedAllocator<U, std::alignment_of_v<U>>;
  };

  AlignedAllocator() noexcept = default;

  template<typename U, std::size_t other_alignment>
  explicit AlignedAllocator(
    const AlignedAllocator<U, other_alignment>&) noexcept
  {}

  [[nodiscard]] value_type* allocate(const size_type n) const
  {
    auto ptr{ allocateAligned<value_type>(alignment, n) };
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

template<typename T,
         std::size_t alignment_t,
         typename U,
         std::size_t alignment_u>
[[nodiscard]] constexpr bool
operator==(const AlignedAllocator<T, alignment_t>&,
           const AlignedAllocator<U, alignment_u>&) noexcept
{
  return true;
}

template<typename T,
         std::size_t alignment_t,
         typename U,
         std::size_t alignment_u>
[[nodiscard]] constexpr bool
operator!=(const AlignedAllocator<T, alignment_t>&,
           const AlignedAllocator<U, alignment_u>&) noexcept
{
  return false;
}

} // namespace FV::Memory
