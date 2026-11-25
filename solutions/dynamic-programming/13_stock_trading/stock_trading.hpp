// Problem 13: stock trading state machines. Reference solution.
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

inline std::int64_t max_profit_with_fee(const std::vector<std::int64_t>& prices, std::int64_t fee) {
    constexpr std::int64_t kNegInf = INT64_MIN / 4;
    std::int64_t flat = 0, held = kNegInf;
    for (std::int64_t p : prices) {
        std::int64_t new_flat = std::max(flat, held + p - fee);
        std::int64_t new_held = std::max(held, flat - p);
        flat = new_flat;
        held = new_held;
    }
    return flat;
}

inline std::int64_t max_profit_unlimited(const std::vector<std::int64_t>& prices) {
    return max_profit_with_fee(prices, 0);
}

inline std::int64_t max_profit_with_cooldown(const std::vector<std::int64_t>& prices) {
    constexpr std::int64_t kNegInf = INT64_MIN / 4;
    std::int64_t flat = 0, held = kNegInf, cool = kNegInf;
    for (std::int64_t p : prices) {
        std::int64_t new_flat = std::max(flat, cool);
        std::int64_t new_held = std::max(held, flat - p);
        std::int64_t new_cool = held + p;
        flat = new_flat;
        held = new_held;
        cool = new_cool;
    }
    return std::max(flat, cool);
}

inline std::int64_t max_profit_k_trades(const std::vector<std::int64_t>& prices, std::int64_t k) {
    const std::int64_t n = static_cast<std::int64_t>(prices.size());
    if (k >= n / 2) return max_profit_unlimited(prices);  // the limit cannot bind
    constexpr std::int64_t kNegInf = INT64_MIN / 4;
    std::vector<std::int64_t> flat(k + 1, 0), held(k + 1, kNegInf);
    for (std::int64_t p : prices)
        for (std::int64_t t = k; t >= 1; --t) {  // downward: flat[t-1] is still yesterday's
            flat[t] = std::max(flat[t], held[t] + p);
            held[t] = std::max(held[t], flat[t - 1] - p);
        }
    return flat[k];
}

}  // namespace dp
