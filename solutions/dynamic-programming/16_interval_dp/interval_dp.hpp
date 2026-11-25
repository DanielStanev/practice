// Problem 16: interval DP with split points. Reference solution.
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace dp {

namespace detail {
// C and split are (n+1) x (n+1) flat tables over 1-based matrix indices.
inline void matrix_chain_tables(const std::vector<int>& dims, std::vector<std::int64_t>& C, std::vector<int>& split) {
    const int n = static_cast<int>(dims.size()) - 1;
    const int W = n + 1;
    C.assign(W * W, 0);
    split.assign(W * W, 0);
    for (int len = 2; len <= n; ++len)
        for (int i = 1; i + len - 1 <= n; ++i) {
            int j = i + len - 1;
            std::int64_t best = INT64_MAX;
            for (int k = i; k < j; ++k) {
                std::int64_t c = C[i * W + k] + C[(k + 1) * W + j] +
                                 static_cast<std::int64_t>(dims[i - 1]) * dims[k] * dims[j];
                if (c < best) best = c, split[i * W + j] = k;
            }
            C[i * W + j] = best;
        }
}

inline std::string print_chain(const std::vector<int>& split, int W, int i, int j) {
    if (i == j) return std::string(1, static_cast<char>('A' + i - 1));
    int k = split[i * W + j];
    return "(" + print_chain(split, W, i, k) + print_chain(split, W, k + 1, j) + ")";
}
}  // namespace detail

inline std::int64_t matrix_chain_cost(const std::vector<int>& dims) {
    std::vector<std::int64_t> C;
    std::vector<int> split;
    detail::matrix_chain_tables(dims, C, split);
    const int n = static_cast<int>(dims.size()) - 1;
    return C[1 * (n + 1) + n];
}

inline std::string matrix_chain_order(const std::vector<int>& dims) {
    std::vector<std::int64_t> C;
    std::vector<int> split;
    detail::matrix_chain_tables(dims, C, split);
    const int n = static_cast<int>(dims.size()) - 1;
    return detail::print_chain(split, n + 1, 1, n);
}

inline std::int64_t burst_balloons(const std::vector<int>& nums) {
    const int n = static_cast<int>(nums.size());
    std::vector<std::int64_t> a(n + 2, 1);
    for (int i = 0; i < n; ++i) a[i + 1] = nums[i];
    const int W = n + 2;
    std::vector<std::int64_t> B(W * W, 0);
    for (int len = 2; len <= n + 1; ++len)
        for (int i = 0; i + len <= n + 1; ++i) {
            int j = i + len;
            std::int64_t best = 0;
            for (int k = i + 1; k < j; ++k)  // k is the last balloon burst in (i, j)
                best = std::max(best, B[i * W + k] + B[k * W + j] + a[i] * a[k] * a[j]);
            B[i * W + j] = best;
        }
    return B[0 * W + n + 1];
}

inline std::int64_t min_cut_cost(std::int64_t length, std::vector<std::int64_t> cuts) {
    cuts.push_back(0);
    cuts.push_back(length);
    std::sort(cuts.begin(), cuts.end());
    const int m = static_cast<int>(cuts.size());
    std::vector<std::int64_t> S(m * m, 0);
    for (int len = 2; len < m; ++len)
        for (int i = 0; i + len < m; ++i) {
            int j = i + len;
            std::int64_t best = INT64_MAX;
            for (int k = i + 1; k < j; ++k) best = std::min(best, S[i * m + k] + S[k * m + j]);
            S[i * m + j] = best + (cuts[j] - cuts[i]);
        }
    return S[0 * m + m - 1];
}

}  // namespace dp
