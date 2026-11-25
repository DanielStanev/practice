// Problem 09: 0/1 knapsack.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

struct Item {
    std::int64_t weight;
    std::int64_t value;
};

// Max total value, each item at most once, total weight <= capacity (<= 100000).
inline std::int64_t knapsack(const std::vector<Item>& items, int capacity) {
    (void)items;
    (void)capacity;
    throw std::logic_error("TODO: knapsack");
}

// Indices (increasing) of a subset achieving knapsack(items, capacity).
inline std::vector<int> knapsack_items(const std::vector<Item>& items, int capacity) {
    (void)items;
    (void)capacity;
    throw std::logic_error("TODO: knapsack_items");
}

// Can nums be split into two groups with equal sums?
inline bool can_partition_equal(const std::vector<int>& nums) {
    (void)nums;
    throw std::logic_error("TODO: can_partition_equal");
}

// Same as knapsack, but weights and capacity go up to 1e9 while each value <= 1000, n <= 100.
inline std::int64_t knapsack_huge_capacity(const std::vector<Item>& items, std::int64_t capacity) {
    (void)items;
    (void)capacity;
    throw std::logic_error("TODO: knapsack_huge_capacity");
}

}  // namespace dp
