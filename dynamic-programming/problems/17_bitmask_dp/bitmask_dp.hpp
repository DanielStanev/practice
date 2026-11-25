// Problem 17: bitmask DP.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <bit>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

using Matrix = std::vector<std::vector<std::int64_t>>;

// Cheapest tour 0 -> every other city exactly once -> 0. dist may be asymmetric. 1 <= n <= 16.
inline std::int64_t tsp_cost(const Matrix& dist) {
    (void)dist;
    throw std::logic_error("TODO: tsp_cost");
}

// One optimal tour: n + 1 city indices, first and last are 0.
inline std::vector<int> tsp_tour(const Matrix& dist) {
    (void)dist;
    throw std::logic_error("TODO: tsp_tour");
}

// cost[w][j] = cost of giving job j to worker w. Min total of a one-to-one assignment. n <= 20.
inline std::int64_t min_assignment_cost(const Matrix& cost) {
    (void)cost;
    throw std::logic_error("TODO: min_assignment_cost");
}

// Can nums (positive) be split into k non-empty groups with equal sums? n <= 16.
inline bool can_partition_k(const std::vector<int>& nums, int k) {
    (void)nums;
    (void)k;
    throw std::logic_error("TODO: can_partition_k");
}

}  // namespace dp
