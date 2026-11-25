// Problem 04: house robber.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

// Max sum of non-adjacent elements. Empty -> 0.
inline std::int64_t rob_line(const std::vector<std::int64_t>& money) {
    (void)money;
    throw std::logic_error("TODO: rob_line");
}

// Same, but money.front() and money.back() are adjacent (a circle).
inline std::int64_t rob_circle(const std::vector<std::int64_t>& money) {
    (void)money;
    throw std::logic_error("TODO: rob_circle");
}

// Pick a value x, earn x for every occurrence, values x-1 and x+1 become forbidden.
// 1 <= nums[i] <= 100000.
inline std::int64_t delete_and_earn(const std::vector<int>& nums) {
    (void)nums;
    throw std::logic_error("TODO: delete_and_earn");
}

}  // namespace dp
