// Problem 10: longest increasing subsequence. Reference solution.
#pragma once

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dp {

inline int lis_length_quadratic(const std::vector<int>& a) {
    const int n = static_cast<int>(a.size());
    std::vector<int> len(n, 1);
    int best = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j)
            if (a[j] < a[i]) len[i] = std::max(len[i], len[j] + 1);
        best = std::max(best, len[i]);
    }
    return best;
}

inline int lis_length(const std::vector<int>& a) {
    std::vector<int> tails;
    for (int x : a) {
        auto it = std::lower_bound(tails.begin(), tails.end(), x);
        if (it == tails.end()) tails.push_back(x);
        else *it = x;
    }
    return static_cast<int>(tails.size());
}

inline std::vector<int> lis_sequence(const std::vector<int>& a) {
    std::vector<int> tails, tail_idx, parent(a.size(), -1);
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
        auto k = std::lower_bound(tails.begin(), tails.end(), a[i]) - tails.begin();
        if (k > 0) parent[i] = tail_idx[k - 1];
        if (k == static_cast<long>(tails.size())) {
            tails.push_back(a[i]);
            tail_idx.push_back(i);
        } else {
            tails[k] = a[i];
            tail_idx[k] = i;
        }
    }
    std::vector<int> seq;
    for (int i = tail_idx.empty() ? -1 : tail_idx.back(); i != -1; i = parent[i]) seq.push_back(a[i]);
    std::reverse(seq.begin(), seq.end());
    return seq;
}

inline int max_envelopes(std::vector<std::pair<int, int>> envelopes) {
    std::sort(envelopes.begin(), envelopes.end(), [](const auto& x, const auto& y) {
        return x.first != y.first ? x.first < y.first : x.second > y.second;
    });
    std::vector<int> heights;
    heights.reserve(envelopes.size());
    for (const auto& e : envelopes) heights.push_back(e.second);
    return lis_length(heights);
}

}  // namespace dp
