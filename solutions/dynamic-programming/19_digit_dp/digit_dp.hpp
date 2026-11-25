// Problem 19: digit DP. Reference solution.
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>

namespace dp {

namespace detail {

// [0, N] with digit sum exactly s.
inline std::int64_t digit_sum_upto(std::int64_t N, int s) {
    if (N < 0) return 0;
    const std::string dig = std::to_string(N);
    const int len = static_cast<int>(dig.size());
    static constexpr int kMaxSum = 9 * 19 + 1;
    if (s >= kMaxSum) return 0;
    std::int64_t memo[20][kMaxSum];
    std::memset(memo, -1, sizeof memo);  // only for loose states
    auto go = [&](auto& self, int pos, int sum, bool tight) -> std::int64_t {
        if (sum > s) return 0;
        if (pos == len) return sum == s ? 1 : 0;
        if (!tight && memo[pos][sum] >= 0) return memo[pos][sum];
        int limit = tight ? dig[pos] - '0' : 9;
        std::int64_t total = 0;
        for (int d = 0; d <= limit; ++d) total += self(self, pos + 1, sum + d, tight && d == limit);
        if (!tight) memo[pos][sum] = total;
        return total;
    };
    return go(go, 0, 0, true);
}

// [0, N] with no two equal adjacent digits. prev = 10 means "not started".
inline std::int64_t no_adjacent_upto(std::int64_t N) {
    if (N < 0) return 0;
    const std::string dig = std::to_string(N);
    const int len = static_cast<int>(dig.size());
    std::int64_t memo[20][11];
    std::memset(memo, -1, sizeof memo);
    auto go = [&](auto& self, int pos, int prev, bool tight) -> std::int64_t {
        if (pos == len) return 1;  // includes the all-padding number 0
        if (!tight && memo[pos][prev] >= 0) return memo[pos][prev];
        int limit = tight ? dig[pos] - '0' : 9;
        std::int64_t total = 0;
        for (int d = 0; d <= limit; ++d) {
            bool started = prev != 10 || d != 0;
            if (started && d == prev) continue;
            total += self(self, pos + 1, started ? d : 10, tight && d == limit);
        }
        if (!tight) memo[pos][prev] = total;
        return total;
    };
    return go(go, 0, 10, true);
}

// Occurrences of digit d over [0, N]. Each state returns {numbers, occurrences}.
inline std::int64_t occurrences_upto(std::int64_t N, int d) {
    if (N < 0) return 0;
    const std::string dig = std::to_string(N);
    const int len = static_cast<int>(dig.size());
    using P = std::pair<std::int64_t, std::int64_t>;
    std::array<std::array<P, 2>, 20> memo;
    std::array<std::array<bool, 2>, 20> known{};
    auto go = [&](auto& self, int pos, bool started, bool tight) -> P {
        if (pos == len) return {1, 0};
        if (!tight && known[pos][started]) return memo[pos][started];
        int limit = tight ? dig[pos] - '0' : 9;
        P total{0, 0};
        for (int x = 0; x <= limit; ++x) {
            bool now_started = started || x != 0;
            P sub = self(self, pos + 1, now_started, tight && x == limit);
            total.first += sub.first;
            total.second += sub.second;
            if (now_started && x == d) total.second += sub.first;  // x is written in every completion
        }
        if (!tight) memo[pos][started] = total, known[pos][started] = true;
        return total;
    };
    std::int64_t occ = go(go, 0, false, true).second;
    if (d == 0) occ += 1;  // the number 0 is written "0", which padding never counts
    return occ;
}

}  // namespace detail

inline std::int64_t count_digit_sum(std::int64_t lo, std::int64_t hi, int s) {
    return detail::digit_sum_upto(hi, s) - detail::digit_sum_upto(lo - 1, s);
}

inline std::int64_t count_no_adjacent_equal(std::int64_t lo, std::int64_t hi) {
    return detail::no_adjacent_upto(hi) - detail::no_adjacent_upto(lo - 1);
}

inline std::int64_t count_digit_occurrences(std::int64_t lo, std::int64_t hi, int d) {
    return detail::occurrences_upto(hi, d) - detail::occurrences_upto(lo - 1, d);
}

}  // namespace dp
