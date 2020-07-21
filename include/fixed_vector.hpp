#pragma once

#include "memory/default_allocator.hpp"

#include <iterator>
#include <memory>
#include <stdexcept>

namespace FV {

template<typename T, typename Allocator = Memory::DefaultAllocator<T>>
class FixedVector
{
public:
  using value_type = T;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer =
    typename std::allocator_traits<Allocator>::const_pointer;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // Constructors
  explicit FixedVector(size_type num_elements,
                       const Allocator& allocator = Allocator{});

  FixedVector(size_type num_elements,
              const value_type& default_value,
              const Allocator& allocator = Allocator{});

  template<typename InputIterator>
  FixedVector(InputIterator first,
              std::enable_if_t<!std::is_same_v<InputIterator, value_type>,
                               InputIterator> last,
              const Allocator& allocator = Allocator{});

  FixedVector(const FixedVector& other);
  FixedVector(FixedVector&& other) noexcept;

  FixedVector& operator=(const FixedVector&) = delete;
  FixedVector& operator=(FixedVector&&) noexcept = delete;

  ~FixedVector();

  allocator_type get_allocator() const { return memory_allocator; }

  // Index based access
  [[nodiscard]] reference at(const size_type index)
  {
    checkIndex(index);
    return storage_start[index];
  }

  [[nodiscard]] const_reference at(const size_type index) const
  {
    checkIndex(index);
    return storage_start[index];
  }

  [[nodiscard]] reference operator[](const size_type index)
  {
    return storage_start[index];
  }

  [[nodiscard]] const_reference operator[](const size_type index) const
  {
    return storage_start[index];
  }

  // Assign the same value to all elements
  void assign(const value_type& value)
  {
    for (iterator it{ begin() }; it != end(); ++it) {
      *it = value;
    }
  }

  // Destroy all created elements
  void clear()
  {
    for (iterator it{ begin() }; it != end(); ++it) {
      std::destroy_at(it);
    }
    next_free_element = storage_start;
  }

  // Insert an element at the end
  template<typename... Args>
  reference emplace_back(Args&&... args)
  {
    if (next_free_element == storage_end) {
      throw std::out_of_range{ "Adding element after end of FixedVector" };
    }
    return emplace(next_free_element++, std::forward<Args>(args)...);
  }

  reference push_back(const value_type& element)
  {
    return emplace_back(element);
  }

  reference push_back(value_type&& element)
  {
    return emplace_back(std::move(element));
  }

  // Access data
  [[nodiscard]] pointer data() { return storage_start; }
  [[nodiscard]] const_pointer data() const { return storage_start; }

  // Iterators
  [[nodiscard]] iterator begin() { return data(); }
  [[nodiscard]] const_iterator begin() const { return data(); }
  [[nodiscard]] const_iterator cbegin() const { return data(); }

  [[nodiscard]] iterator end() { return next_free_element; }
  [[nodiscard]] const_iterator end() const { return next_free_element; }
  [[nodiscard]] const_iterator cend() const { return next_free_element; }

  [[nodiscard]] reverse_iterator rbegin() { return reverse_iterator{ end() }; }
  [[nodiscard]] const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator{ end() };
  }
  [[nodiscard]] const_reverse_iterator crbegin() const
  {
    return const_reverse_iterator{ cend() };
  }

  [[nodiscard]] reverse_iterator rend() { return reverse_iterator{ begin() }; }
  [[nodiscard]] const_reverse_iterator rend() const
  {
    return const_reverse_iterator{ begin() };
  }
  [[nodiscard]] const_reverse_iterator crend() const
  {
    return const_reverse_iterator{ cbegin() };
  }

  // Current size of the vector
  [[nodiscard]] size_type size() const
  {
    return static_cast<size_type>(std::distance(begin(), end()));
  }

  // Maximum capacity of the vector
  [[nodiscard]] size_type capacity() const
  {
    return static_cast<size_type>(std::distance(storage_start, storage_end));
  }

  [[nodiscard]] bool empty() const { return begin() == end(); }
  [[nodiscard]] bool full() const { return end() == storage_end; }

  [[nodiscard]] reference front() { return begin()[0]; }
  [[nodiscard]] const_reference front() const { return cbegin()[0]; }

  [[nodiscard]] reference back() { return *std::prev(end()); }
  [[nodiscard]] const_reference back() const { return *std::prev(cend()); }

  void swap(FixedVector& other)
  {
    std::swap(memory_allocator, other.memory_allocator);
    std::swap(storage_start, other.storage_start);
    std::swap(next_free_element, other.next_free_element);
    std::swap(storage_end, other.storage_end);
  }

private:
  // Insert an element at the given position
  template<typename... Args>
  reference emplace(iterator position, Args&&... args)
  {
    new (position) T{ std::forward<Args>(args)... };
    return *position;
  }

  void checkIndex(const size_type index) const
  {
    if (index >= size()) {
      throw std::out_of_range{ "Index out of range in FixedVector" };
    }
  }

  Allocator memory_allocator;

  pointer storage_start;
  pointer next_free_element;
  pointer storage_end;
};

template<typename T, typename Allocator>
FixedVector<T, Allocator>::FixedVector(const size_type num_elements,
                                       const Allocator& allocator)
  : memory_allocator{ allocator }
  , storage_start{ memory_allocator.allocate(num_elements) }
  , next_free_element{ storage_start }
  , storage_end{ storage_start + static_cast<difference_type>(num_elements) }
{}

template<typename T, typename Allocator>
FixedVector<T, Allocator>::FixedVector(const size_type num_elements,
                                       const value_type& default_value,
                                       const Allocator& allocator)
  : FixedVector{ num_elements, allocator }
{
  for (iterator it{ begin() }; it != storage_end; ++it) {
    push_back(default_value);
  }
}

template<typename T, typename Allocator>
template<typename InputIterator>
FixedVector<T, Allocator>::FixedVector(
  InputIterator first,
  std::enable_if_t<!std::is_same_v<InputIterator, value_type>, InputIterator>
    last,
  const Allocator& allocator)
  : FixedVector{ static_cast<size_type>(std::distance(first, last)), allocator }
{
  for (; first != last; ++first) {
    emplace_back(*first);
  }
}

template<typename T, typename Allocator>
FixedVector<T, Allocator>::FixedVector(const FixedVector& other)
  : FixedVector{
    other.begin(),
    other.end(),
    std::allocator_traits<Allocator>::select_on_container_copy_construction(
      other.get_allocator())
  }
{}

template<typename T, typename Allocator>
FixedVector<T, Allocator>::FixedVector(FixedVector&& other) noexcept
  : memory_allocator{ std::move(other.memory_allocator) }
  , storage_start{ std::exchange(other.storage_start, nullptr) }
  , next_free_element{ std::exchange(other.next_free_element, nullptr) }
  , storage_end{ std::exchange(other.storage_end, nullptr) }
{}

template<typename T, typename Allocator>
FixedVector<T, Allocator>::~FixedVector()
{
  if (begin()) {
    clear();
    memory_allocator.deallocate(
      storage_start,
      static_cast<typename std::allocator_traits<Allocator>::size_type>(
        size()));
    storage_start = nullptr;
  }
}

} // namespace FV
