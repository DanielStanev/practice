// Problem 11: longest common subsequence / substring. Reference solution.
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace dp {

inline int lcs_length(std::string_view a, std::string_view b) {
    if (a.size() < b.size()) std::swap(a, b);  // rows over the longer one, row length = shorter
    const std::size_t m = b.size();
    std::vector<int> prev(m + 1, 0), cur(m + 1, 0);
    for (char ca : a) {
        for (std::size_t j = 1; j <= m; ++j)
            cur[j] = ca == b[j - 1] ? prev[j - 1] + 1 : std::max(prev[j], cur[j - 1]);
        std::swap(prev, cur);
    }
    return prev[m];
}

inline std::string lcs_string(std::string_view a, std::string_view b) {
    const std::size_t n = a.size(), m = b.size(), W = m + 1;
    std::vector<std::uint16_t> L((n + 1) * W, 0);
    for (std::size_t i = 1; i <= n; ++i)
        for (std::size_t j = 1; j <= m; ++j)
            L[i * W + j] = a[i - 1] == b[j - 1] ? L[(i - 1) * W + j - 1] + 1
                                                : std::max(L[(i - 1) * W + j], L[i * W + j - 1]);
    std::string out;
    std::size_t i = n, j = m;
    while (i > 0 && j > 0) {
        if (a[i - 1] == b[j - 1]) {
            out.push_back(a[i - 1]);
            --i, --j;
        } else if (L[(i - 1) * W + j] >= L[i * W + j - 1]) {
            --i;
        } else {
            --j;
        }
    }
    std::reverse(out.begin(), out.end());
    return out;
}

inline int longest_common_substring(std::string_view a, std::string_view b) {
    const std::size_t m = b.size();
    std::vector<int> prev(m + 1, 0), cur(m + 1, 0);
    int best = 0;
    for (char ca : a) {
        for (std::size_t j = 1; j <= m; ++j) {
            cur[j] = ca == b[j - 1] ? prev[j - 1] + 1 : 0;
            best = std::max(best, cur[j]);
        }
        std::swap(prev, cur);
    }
    return best;
}

}  // namespace dp
