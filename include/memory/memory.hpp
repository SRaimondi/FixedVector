#pragma once

#include <cassert>
#include <new>

#if defined(_MSC_VER)
#include <malloc.h>
#else
#include <cstdlib>
#endif

namespace FV::Memory {
namespace Internal {

[[nodiscard]] constexpr std::size_t
roundAllocationSize(const std::size_t alignment,
                    const std::size_t size_in_bytes) noexcept
{
  return ((alignment + size_in_bytes - 1) / alignment) * alignment;
}

[[nodiscard]] constexpr bool
isPowerOfTwo(const std::size_t alignment) noexcept
{
  return (alignment != 0) && ((alignment & (alignment - 1)) == 0);
}

} // namespace Internal

template<typename T>
[[nodiscard]] inline T*
allocateAligned(const std::size_t alignment, const std::size_t num_elements)
{
  assert(Internal::isPowerOfTwo(alignment));

  const auto rounded_allocation_size{ Internal::roundAllocationSize(
    alignment, num_elements * sizeof(T)) };
#if defined(__APPLE__)
  T* ptr{ static_cast<T*>(aligned_alloc(alignment, rounded_allocation_size)) };
#elif defined(_MSC_VER)
  T* ptr{ static_cast<T*>(
    _aligned_malloc(rounded_allocation_size, alignment)) };
#else
  T* ptr{ static_cast<T*>(
    std::aligned_alloc(alignment, rounded_allocation_size)) };
#endif

  if (ptr == nullptr) {
    throw std::bad_alloc();
  }

  return ptr;
}

template<typename T>
inline void
freeAligned(T* ptr) noexcept
{
#if defined(_MSC_VER)
  _aligned_free(static_cast<void*>(ptr));
#else
  std::free(static_cast<void*>(ptr));
#endif
}

} // namespace FV::Memory
