// Problem 13: stock trading state machines.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

// Any number of buy/sell round trips, holding at most one share.
inline std::int64_t max_profit_unlimited(const std::vector<std::int64_t>& prices) {
    (void)prices;
    throw std::logic_error("TODO: max_profit_unlimited");
}

// Same, but each completed round trip costs fee.
inline std::int64_t max_profit_with_fee(const std::vector<std::int64_t>& prices, std::int64_t fee) {
    (void)prices;
    (void)fee;
    throw std::logic_error("TODO: max_profit_with_fee");
}

// Same as unlimited, but you cannot buy on the day right after a sell.
inline std::int64_t max_profit_with_cooldown(const std::vector<std::int64_t>& prices) {
    (void)prices;
    throw std::logic_error("TODO: max_profit_with_cooldown");
}

// At most k round trips.
inline std::int64_t max_profit_k_trades(const std::vector<std::int64_t>& prices, std::int64_t k) {
    (void)prices;
    (void)k;
    throw std::logic_error("TODO: max_profit_k_trades");
}

}  // namespace dp
