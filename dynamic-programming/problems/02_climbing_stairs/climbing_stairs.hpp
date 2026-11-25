// Problem 02: climbing stairs.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

inline constexpr std::int64_t kMod = 1'000'000'007;

// Moves of 1 or 2 steps. 0 <= n <= 90. count_ways(0) == 1.
inline std::uint64_t count_ways(int n) {
    (void)n;
    throw std::logic_error("TODO: count_ways");
}

// Moves from `steps` (distinct, positive). Result modulo kMod. 0 <= n <= 200000.
inline std::int64_t count_ways_with_steps(int n, const std::vector<int>& steps) {
    (void)n;
    (void)steps;
    throw std::logic_error("TODO: count_ways_with_steps");
}

// Moves of 1 or 2 steps, never two 2-steps in a row. 0 <= n <= 80.
inline std::uint64_t count_ways_no_double_two(int n) {
    (void)n;
    throw std::logic_error("TODO: count_ways_no_double_two");
}

}  // namespace dp
