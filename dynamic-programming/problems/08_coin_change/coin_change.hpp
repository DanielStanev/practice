// Problem 08: coin change.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

inline constexpr std::int64_t kMod = 1'000'000'007;

// Fewest coins (unlimited supply) summing to amount, or -1 if impossible.
inline int min_coins(const std::vector<int>& coins, int amount) {
    (void)coins;
    (void)amount;
    throw std::logic_error("TODO: min_coins");
}

// Number of multisets of coins summing to amount (order does not matter), modulo kMod.
inline std::int64_t count_combinations(const std::vector<int>& coins, int amount) {
    (void)coins;
    (void)amount;
    throw std::logic_error("TODO: count_combinations");
}

// Number of ordered sequences of coins summing to amount, modulo kMod.
inline std::int64_t count_permutations(const std::vector<int>& coins, int amount) {
    (void)coins;
    (void)amount;
    throw std::logic_error("TODO: count_permutations");
}

}  // namespace dp
