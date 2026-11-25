// Problem 20: DP optimizations.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

// Max sum of a non-empty subsequence whose consecutive chosen indices differ by at most k.
inline std::int64_t max_constrained_sum(const std::vector<std::int64_t>& nums, int k) {
    (void)nums;
    (void)k;
    throw std::logic_error("TODO: max_constrained_sum");
}

// Split a (non-negative) into exactly k non-empty contiguous groups minimizing the sum of squared group sums.
inline std::int64_t min_partition_cost(const std::vector<std::int64_t>& a, int k) {
    (void)a;
    (void)k;
    throw std::logic_error("TODO: min_partition_cost");
}

// h strictly increasing. A jump j -> i (j < i) costs (h[i] - h[j])^2 + C. Min cost from stone 0 to the last.
inline std::int64_t frog_squared(const std::vector<std::int64_t>& h, std::int64_t C) {
    (void)h;
    (void)C;
    throw std::logic_error("TODO: frog_squared");
}

}  // namespace dp
