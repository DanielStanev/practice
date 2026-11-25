// Problem 07: grid paths.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace dp {

inline constexpr std::int64_t kMod = 1'000'000'007;

// Number of right/down paths from top-left to bottom-right. 1 <= rows, cols <= 30.
inline std::uint64_t unique_paths(int rows, int cols) {
    (void)rows;
    (void)cols;
    throw std::logic_error("TODO: unique_paths");
}

// '.' free, '#' blocked. Result modulo kMod.
inline std::int64_t unique_paths_with_obstacles(const std::vector<std::string>& grid) {
    (void)grid;
    throw std::logic_error("TODO: unique_paths_with_obstacles");
}

// Cheapest right/down path, summing every visited cell including both ends.
inline std::int64_t min_path_sum(const std::vector<std::vector<std::int64_t>>& cost) {
    (void)cost;
    throw std::logic_error("TODO: min_path_sum");
}

// Side of the largest square containing only '1'.
inline int largest_square(const std::vector<std::string>& grid) {
    (void)grid;
    throw std::logic_error("TODO: largest_square");
}

}  // namespace dp
