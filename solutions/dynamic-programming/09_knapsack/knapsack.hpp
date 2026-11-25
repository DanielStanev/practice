// Problem 09: 0/1 knapsack. Reference solution.
#pragma once

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace dp {

struct Item {
    std::int64_t weight;
    std::int64_t value;
};

inline std::int64_t knapsack(const std::vector<Item>& items, int capacity) {
    std::vector<std::int64_t> best(capacity + 1, 0);
    for (const auto& it : items)
        for (std::int64_t w = capacity; w >= it.weight; --w)  // downward: each item once
            best[w] = std::max(best[w], best[w - it.weight] + it.value);
    return best[capacity];
}

inline std::vector<int> knapsack_items(const std::vector<Item>& items, int capacity) {
    const int n = static_cast<int>(items.size());
    const int W = capacity + 1;
    std::vector<std::int64_t> best(static_cast<std::size_t>(n + 1) * W, 0);  // flat (n+1) x (capacity+1)
    auto at = [&](int i, std::int64_t w) -> std::int64_t& { return best[static_cast<std::size_t>(i) * W + w]; };
    for (int i = 1; i <= n; ++i)
        for (int w = 0; w <= capacity; ++w) {
            at(i, w) = at(i - 1, w);
            const auto& it = items[i - 1];
            if (it.weight <= w) at(i, w) = std::max(at(i, w), at(i - 1, w - it.weight) + it.value);
        }
    std::vector<int> chosen;
    std::int64_t w = capacity;
    for (int i = n; i >= 1; --i)
        if (at(i, w) != at(i - 1, w)) {
            chosen.push_back(i - 1);
            w -= items[i - 1].weight;
        }
    std::reverse(chosen.begin(), chosen.end());
    return chosen;
}

inline bool can_partition_equal(const std::vector<int>& nums) {
    int total = std::accumulate(nums.begin(), nums.end(), 0);
    if (total % 2 != 0) return false;
    std::bitset<200 * 1000 / 2 + 1> reach;  // reach[s]: some subset sums to s
    reach[0] = 1;
    for (int x : nums) reach |= reach << x;
    return reach[total / 2];
}

inline std::int64_t knapsack_huge_capacity(const std::vector<Item>& items, std::int64_t capacity) {
    constexpr std::int64_t kInf = INT64_MAX / 4;
    std::int64_t total_value = 0;
    for (const auto& it : items) total_value += it.value;
    std::vector<std::int64_t> min_weight(total_value + 1, kInf);
    min_weight[0] = 0;
    for (const auto& it : items)
        for (std::int64_t v = total_value; v >= it.value; --v)
            min_weight[v] = std::min(min_weight[v], min_weight[v - it.value] + it.weight);
    for (std::int64_t v = total_value; v >= 0; --v)
        if (min_weight[v] <= capacity) return v;
    return 0;
}

}  // namespace dp
