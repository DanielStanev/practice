// Problem 02: climbing stairs. Reference solution.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

inline constexpr std::int64_t kMod = 1'000'000'007;

inline std::uint64_t count_ways(int n) {
    std::uint64_t prev = 1, cur = 1;  // ways[k-1], ways[k] for k = 1 (ways[0] = 1)
    if (n == 0) return 1;
    for (int k = 2; k <= n; ++k) {
        std::uint64_t next = prev + cur;
        prev = cur;
        cur = next;
    }
    return cur;
}

inline std::int64_t count_ways_with_steps(int n, const std::vector<int>& steps) {
    std::vector<std::int64_t> ways(n + 1, 0);
    ways[0] = 1;
    for (int k = 1; k <= n; ++k) {
        std::int64_t total = 0;
        for (int s : steps)
            if (s <= k) total = (total + ways[k - s]) % kMod;
        ways[k] = total;
    }
    return ways[n];
}

inline std::uint64_t count_ways_no_double_two(int n) {
    // end1[k]: last move was a 1-step (step 0 counts as this), end2[k]: last move was a 2-step.
    std::vector<std::uint64_t> end1(n + 1, 0), end2(n + 1, 0);
    end1[0] = 1;
    for (int k = 1; k <= n; ++k) {
        end1[k] = end1[k - 1] + end2[k - 1];
        if (k >= 2) end2[k] = end1[k - 2];
    }
    return end1[n] + end2[n];
}

}  // namespace dp
