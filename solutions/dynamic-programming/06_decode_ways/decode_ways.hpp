// Problem 06: decode ways, word break, fewest words. Reference solution.
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace dp {

inline constexpr std::int64_t kMod = 1'000'000'007;

inline std::int64_t num_decodings(std::string_view s) {
    const std::size_t n = s.size();
    std::vector<std::int64_t> ways(n + 1, 0);
    ways[0] = 1;
    for (std::size_t i = 1; i <= n; ++i) {
        if (s[i - 1] != '0') ways[i] = ways[i - 1];
        if (i >= 2) {
            int two = (s[i - 2] - '0') * 10 + (s[i - 1] - '0');
            if (s[i - 2] != '0' && two <= 26) ways[i] = (ways[i] + ways[i - 2]) % kMod;
        }
    }
    return ways[n];
}

namespace detail {
// best[i] = fewest words covering s[0..i), or -1.
inline std::vector<int> word_table(std::string_view s, const std::vector<std::string>& dict) {
    std::unordered_set<std::string_view> words(dict.begin(), dict.end());
    std::size_t max_len = 0;
    for (const auto& w : dict) max_len = std::max(max_len, w.size());
    std::vector<int> best(s.size() + 1, -1);
    best[0] = 0;
    for (std::size_t i = 1; i <= s.size(); ++i)
        for (std::size_t len = 1; len <= std::min(max_len, i); ++len) {
            int prev = best[i - len];
            if (prev < 0 || !words.count(s.substr(i - len, len))) continue;
            if (best[i] < 0 || prev + 1 < best[i]) best[i] = prev + 1;
        }
    return best;
}
}  // namespace detail

inline bool word_break(std::string_view s, const std::vector<std::string>& dict) {
    return detail::word_table(s, dict).back() >= 0;
}

inline int min_words(std::string_view s, const std::vector<std::string>& dict) {
    return detail::word_table(s, dict).back();
}

}  // namespace dp
