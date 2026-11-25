// Problem 14: palindromes (interval DP). Reference solution.
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace dp {

namespace detail {
// Flat n x n table, pal[i * n + j] = 1 iff s[i..j] is a palindrome (i <= j). Filled by length.
inline std::vector<std::uint8_t> palindrome_table(std::string_view s) {
    const std::size_t n = s.size();
    std::vector<std::uint8_t> pal(n * n, 0);
    for (std::size_t len = 1; len <= n; ++len)
        for (std::size_t i = 0; i + len <= n; ++i) {
            std::size_t j = i + len - 1;
            pal[i * n + j] = s[i] == s[j] && (len <= 2 || pal[(i + 1) * n + (j - 1)]);
        }
    return pal;
}
}  // namespace detail

inline std::string longest_palindromic_substring(std::string_view s) {
    const std::size_t n = s.size();
    auto pal = detail::palindrome_table(s);
    std::size_t best_i = 0, best_len = 0;
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i; j < n; ++j)
            if (pal[i * n + j] && j - i + 1 > best_len) best_i = i, best_len = j - i + 1;
    return std::string(s.substr(best_i, best_len));
}

inline std::int64_t count_palindromic_substrings(std::string_view s) {
    auto pal = detail::palindrome_table(s);
    return std::count(pal.begin(), pal.end(), std::uint8_t{1});
}

inline int longest_palindromic_subsequence(std::string_view s) {
    const int n = static_cast<int>(s.size());
    if (n == 0) return 0;
    std::vector<int> next(n, 0), cur(n, 0);  // next = row i+1, cur = row i
    for (int i = n - 1; i >= 0; --i) {
        cur[i] = 1;
        for (int j = i + 1; j < n; ++j)
            cur[j] = s[i] == s[j] ? (j - i >= 2 ? next[j - 1] : 0) + 2 : std::max(next[j], cur[j - 1]);
        std::swap(next, cur);
    }
    return next[n - 1];
}

inline int min_palindrome_cuts(std::string_view s) {
    const std::size_t n = s.size();
    auto pal = detail::palindrome_table(s);
    std::vector<int> pieces(n + 1, 0);
    for (std::size_t j = 1; j <= n; ++j) {
        pieces[j] = static_cast<int>(j);  // worst case: every char alone
        for (std::size_t i = 0; i < j; ++i)
            if (pal[i * n + (j - 1)]) pieces[j] = std::min(pieces[j], pieces[i] + 1);
    }
    return pieces[n] - 1;
}

}  // namespace dp
