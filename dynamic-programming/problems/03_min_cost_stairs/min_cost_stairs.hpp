// Problem 03: min-cost stairs and the frog.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

// Start on step 0 or 1, pay cost[i] for each step you stand on, move +1 or +2,
// finish at index cost.size() (free). Returns the minimum total cost.
inline std::int64_t min_cost_climb(const std::vector<std::int64_t>& cost) {
    (void)cost;
    throw std::logic_error("TODO: min_cost_climb");
}

// The indices you stand on in one cheapest route, in increasing order.
inline std::vector<int> min_cost_path(const std::vector<std::int64_t>& cost) {
    (void)cost;
    throw std::logic_error("TODO: min_cost_path");
}

// Frog on stone 0 must reach the last stone. A jump from i to j (i < j <= i + k)
// costs |h[i] - h[j]|. Returns the minimum total cost.
inline std::int64_t frog_min_energy(const std::vector<std::int64_t>& h, int k) {
    (void)h;
    (void)k;
    throw std::logic_error("TODO: frog_min_energy");
}

}  // namespace dp
