// Problem 05: maximum subarray, one trade, maximum product. Reference solution.
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dp {

inline std::pair<std::size_t, std::size_t> max_subarray_range(const std::vector<std::int64_t>& a) {
    std::int64_t end_here = a[0], best = a[0];
    std::size_t start = 0;
    std::pair<std::size_t, std::size_t> best_range{0, 1};
    for (std::size_t i = 1; i < a.size(); ++i) {
        if (end_here < 0) {
            end_here = a[i];
            start = i;
        } else {
            end_here += a[i];
        }
        if (end_here > best) {
            best = end_here;
            best_range = {start, i + 1};
        }
    }
    return best_range;
}

inline std::int64_t max_subarray_sum(const std::vector<std::int64_t>& a) {
    std::int64_t end_here = a[0], best = a[0];
    for (std::size_t i = 1; i < a.size(); ++i) {
        end_here = a[i] + std::max<std::int64_t>(0, end_here);
        best = std::max(best, end_here);
    }
    return best;
}

inline std::int64_t max_profit_one_trade(const std::vector<std::int64_t>& prices) {
    std::int64_t best = 0;
    if (prices.empty()) return 0;
    std::int64_t min_so_far = prices[0];
    for (std::size_t i = 1; i < prices.size(); ++i) {
        best = std::max(best, prices[i] - min_so_far);
        min_so_far = std::min(min_so_far, prices[i]);
    }
    return best;
}

inline std::int64_t max_product_subarray(const std::vector<std::int64_t>& a) {
    std::int64_t hi = a[0], lo = a[0], best = a[0];
    for (std::size_t i = 1; i < a.size(); ++i) {
        std::int64_t x = a[i];
        std::int64_t c1 = x * hi, c2 = x * lo;
        hi = std::max({x, c1, c2});
        lo = std::min({x, c1, c2});
        best = std::max(best, hi);
    }
    return best;
}

}  // namespace dp
