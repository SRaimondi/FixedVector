#pragma once

#include <cstdlib>


namespace FV::Memory {
namespace Internal {

[[nodiscard]] constexpr std::size_t
roundAllocationSize(const std::size_t alignment,
                    const std::size_t size_in_bytes) noexcept
{
  return ((alignment + size_in_bytes - 1) / alignment) * alignment;
}

} // namespace Internal

template<typename T>
[[nodiscard]] inline T*
allocateAligned(const std::size_t alignment,
                const std::size_t num_elements) noexcept
{
  const auto rounded_allocation_size{ Internal::roundAllocationSize(
    alignment, num_elements * sizeof(T)) };
#if defined(__APPLE__)
  T* ptr{ static_cast<T*>(aligned_alloc(alignment, rounded_allocation_size)) };
#else
  T* ptr{ static_cast<T*>(
    std::aligned_alloc(alignment, rounded_allocation_size)) };
#endif

  return ptr;
}

} // namespace FV::Memory
