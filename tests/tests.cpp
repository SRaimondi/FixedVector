#include "fixed_vector.hpp"
#include "memory/aligned_allocator.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <array>
#include <string>

TEST_CASE("Testing FixedVector with default allocator", "[vector]")
{
  SECTION("Creating an empty vector of a given size and adding elements")
  {
    constexpr std::size_t SIZE{ 10 };
    FV::FixedVector<int> v{ SIZE };

    // Check size and capacity
    REQUIRE(v.size() == 0);
    REQUIRE(v.empty());
    REQUIRE(v.capacity() == SIZE);

    // Check that access using at() throws
    for (std::size_t i{ 0 }; i != v.capacity(); ++i) {
      REQUIRE_THROWS_AS(v.at(i), std::out_of_range);
    }

    // Check memory is there
    REQUIRE(v.data());

    // Check start and end match
    REQUIRE(v.begin() == v.end());
    REQUIRE(v.rbegin() == v.rend());

    // Add some elements
    for (int i{ 0 }; i != static_cast<int>(SIZE); ++i) {
      v.push_back(i);
    }
    for (std::size_t i{ 0 }; i != SIZE; ++i) {
      REQUIRE(v[i] == static_cast<int>(i));
    }
    REQUIRE(v.size() == SIZE);
    REQUIRE_FALSE(v.empty());

    // Check we cannot add anymore
    REQUIRE_THROWS_AS(v.push_back(101), std::out_of_range);
  }

  SECTION("Creating an empty vector of a given size with a default value")
  {
    constexpr std::size_t SIZE{ 10 };
    constexpr int DEFAULT_VALUE{ 14 };
    FV::FixedVector<int> v{ SIZE, DEFAULT_VALUE };

    // Check size and capacity
    REQUIRE(v.size() == SIZE);
    REQUIRE(v.full());
    REQUIRE_FALSE(v.empty());
    REQUIRE(v.capacity() == SIZE);

    // Check that access using at() does not throw, unless out of range
    for (std::size_t i{ 0 }; i != 2 * v.size(); ++i) {
      if (i < v.size()) {
        REQUIRE_NOTHROW(v.at(i));
      } else {
        REQUIRE_THROWS_AS(v.at(i), std::out_of_range);
      }
    }

    // Check memory is there
    REQUIRE(v.data());

    // Check iterators
    REQUIRE(std::distance(v.begin(), v.end()) == SIZE);
    REQUIRE(std::distance(v.cbegin(), v.cend()) == SIZE);
    REQUIRE(std::distance(v.rbegin(), v.rend()) == SIZE);
    REQUIRE(std::distance(v.crbegin(), v.crend()) == SIZE);

    // Check values
    for (const int e : v) {
      REQUIRE(e == DEFAULT_VALUE);
    }
    REQUIRE(v.front() == DEFAULT_VALUE);
    REQUIRE(v.back() == DEFAULT_VALUE);

    // Check index operator
    for (std::size_t i{ 0 }; i != v.size(); ++i) {
      v[i] = static_cast<int>(i);
    }
    for (std::size_t i{ 0 }; i != v.size(); ++i) {
      REQUIRE(v[i] == static_cast<int>(i));
    }

    // Clear the vector
    v.clear();
    REQUIRE(v.empty());
    REQUIRE(v.size() == 0);
    REQUIRE(v.capacity() == SIZE);
    REQUIRE_THROWS_AS(v.at(0), std::out_of_range);
  }

  SECTION("Creating a vector from a range of elements", "[vector]")
  {
    constexpr std::size_t SIZE{ 10 };
    const std::array<int, SIZE> a{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // Create from range
    const FV::FixedVector<int> v{ a.begin(), a.end() };
    REQUIRE(v.size() == SIZE);

    // Check elements
    for (std::size_t i{ 0 }; i != v.size(); ++i) {
      REQUIRE(v[i] == a[i]);
    }

    // Check reverse looping
    std::size_t a_index{ a.size() - 1 };
    for (auto it{ v.crbegin() }; it != v.crend(); ++it) {
      REQUIRE(a[a_index--] == *it);
    }
  }

  SECTION("Working with a vector of non-trivial type", "[vector]")
  {
    struct Test
    {
      constexpr Test(const int a, const int b)
        : sum{ a + b }
      {}
      int sum;
    };

    constexpr std::size_t SIZE{ 10 };
    FV::FixedVector<Test> v{ SIZE };

    // Add some elements
    for (std::size_t i{ 0 }; i != SIZE; ++i) {
      v.emplace_back(static_cast<int>(2 * i), static_cast<int>(2 * i + 1));
    }
    REQUIRE(v.front().sum == 1);
    REQUIRE(v.back().sum == 37);
    for (std::size_t i{ 0 }; i != SIZE; ++i) {
      REQUIRE(v[i].sum == static_cast<int>(2 * i + 2 * i + 1));
    }

    // Test swapping
    FV::FixedVector<Test> v2{ 2 * SIZE, Test{ 1, 1 } };
    for (const auto& e : v2) {
      REQUIRE(e.sum == 2);
    }
    v.swap(v2);

    for (const auto& e : v) {
      REQUIRE(e.sum == 2);
    }
    for (std::size_t i{ 0 }; i != SIZE; ++i) {
      REQUIRE(v2[i].sum == static_cast<int>(2 * i + 2 * i + 1));
    }
  }

  SECTION("Copying vectors", "[vector]")
  {
    constexpr std::size_t SIZE{ 10 };
    const FV::FixedVector<int> v{ 10, 1 };
    const FV::FixedVector<int> v2{ v };
    REQUIRE(v.size() == SIZE);
    REQUIRE(v2.size() == SIZE);
    for (std::size_t i{ 0 }; i != SIZE; ++i) {
      REQUIRE(v[i] == v2[i]);
    }
  }

  SECTION("Moving vectors", "[vector]")
  {
    constexpr std::size_t SIZE{ 10 };
    FV::FixedVector<int> v{ 10, 1 };
    FV::FixedVector<int> v2{ std::move(v) };
    REQUIRE(v2.size() == SIZE);
    REQUIRE(!v2.empty());
    REQUIRE(v.data() == nullptr);
    for (const int e : v2) {
      REQUIRE(e == 1);
    }
  }

  SECTION("Clear with non trivial type", "[vector]")
  {
    struct Test
    {
      Test()
        : s{ "Testing with a string that is long enough to avoid short string "
             "optimisation" }
      {}
      std::string s;
    };

    constexpr std::size_t SIZE{ 10 };
    FV::FixedVector<Test> v{ SIZE, Test{} };
    v.clear();
    REQUIRE(v.empty());
    REQUIRE(v.capacity() == SIZE);
  }
}