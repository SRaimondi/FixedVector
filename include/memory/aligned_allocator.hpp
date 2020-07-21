#pragma once

#include "memory.hpp"

#include <type_traits>

namespace Cato::Memory {

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

  [[nodiscard]] T* allocate(const size_type n) const
  {
    return allocateAligned<T>(alignment, n);
  }

  void deallocate(T* ptr, const size_type) const noexcept { freeAligned(ptr); }
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

} // namespace Cato::Memory
