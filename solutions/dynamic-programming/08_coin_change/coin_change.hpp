// Problem 08: coin change. Reference solution.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

inline constexpr std::int64_t kMod = 1'000'000'007;

inline int min_coins(const std::vector<int>& coins, int amount) {
    std::vector<int> best(amount + 1, -1);
    best[0] = 0;
    for (int a = 1; a <= amount; ++a)
        for (int c : coins) {
            if (c > a || best[a - c] < 0) continue;
            if (best[a] < 0 || best[a - c] + 1 < best[a]) best[a] = best[a - c] + 1;
        }
    return best[amount];
}

inline std::int64_t count_combinations(const std::vector<int>& coins, int amount) {
    std::vector<std::int64_t> ways(amount + 1, 0);
    ways[0] = 1;
    for (int c : coins)                     // coins outside: each multiset once
        for (int a = c; a <= amount; ++a)   // upward: coin c may be reused
            ways[a] = (ways[a] + ways[a - c]) % kMod;
    return ways[amount];
}

inline std::int64_t count_permutations(const std::vector<int>& coins, int amount) {
    std::vector<std::int64_t> ways(amount + 1, 0);
    ways[0] = 1;
    for (int a = 1; a <= amount; ++a)       // amounts outside: last coin can be anything
        for (int c : coins)
            if (c <= a) ways[a] = (ways[a] + ways[a - c]) % kMod;
    return ways[amount];
}

}  // namespace dp
