// Problem 04: house robber. Reference solution.
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

namespace detail {
// best over the half-open range [first, last).
inline std::int64_t rob_range(const std::vector<std::int64_t>& money, std::size_t first, std::size_t last) {
    std::int64_t two_back = 0, one_back = 0;  // best[i-2], best[i-1]
    for (std::size_t i = first; i < last; ++i) {
        std::int64_t cur = std::max(one_back, money[i] + two_back);
        two_back = one_back;
        one_back = cur;
    }
    return one_back;
}
}  // namespace detail

inline std::int64_t rob_line(const std::vector<std::int64_t>& money) {
    return detail::rob_range(money, 0, money.size());
}

inline std::int64_t rob_circle(const std::vector<std::int64_t>& money) {
    const std::size_t n = money.size();
    if (n == 0) return 0;
    if (n == 1) return money[0];
    return std::max(detail::rob_range(money, 1, n), detail::rob_range(money, 0, n - 1));
}

inline std::int64_t delete_and_earn(const std::vector<int>& nums) {
    if (nums.empty()) return 0;
    int max_v = *std::max_element(nums.begin(), nums.end());
    std::vector<std::int64_t> gain(max_v + 1, 0);
    for (int x : nums) gain[x] += x;
    return rob_line(gain);
}

}  // namespace dp
