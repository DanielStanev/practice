// Problem 05: maximum subarray, one trade, maximum product.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dp {

// Largest sum of a non-empty contiguous subarray. a is non-empty.
inline std::int64_t max_subarray_sum(const std::vector<std::int64_t>& a) {
    (void)a;
    throw std::logic_error("TODO: max_subarray_sum");
}

// {begin, end}, half-open, of a non-empty subarray whose sum is maximal.
inline std::pair<std::size_t, std::size_t> max_subarray_range(const std::vector<std::int64_t>& a) {
    (void)a;
    throw std::logic_error("TODO: max_subarray_range");
}

// Max of prices[j] - prices[i] over i < j, or 0 if no profitable trade exists.
inline std::int64_t max_profit_one_trade(const std::vector<std::int64_t>& prices) {
    (void)prices;
    throw std::logic_error("TODO: max_profit_one_trade");
}

// Largest product of a non-empty contiguous subarray. a is non-empty.
inline std::int64_t max_product_subarray(const std::vector<std::int64_t>& a) {
    (void)a;
    throw std::logic_error("TODO: max_product_subarray");
}

}  // namespace dp
