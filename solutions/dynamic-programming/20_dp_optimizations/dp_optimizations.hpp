// Problem 20: DP optimizations. Reference solution.
#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <stdexcept>
#include <vector>

namespace dp {

inline std::int64_t max_constrained_sum(const std::vector<std::int64_t>& nums, int k) {
    const int n = static_cast<int>(nums.size());
    std::vector<std::int64_t> best(n);
    std::deque<int> window;  // indices with decreasing best[]
    std::int64_t answer = INT64_MIN;
    for (int i = 0; i < n; ++i) {
        while (!window.empty() && window.front() < i - k) window.pop_front();
        std::int64_t prev = window.empty() ? 0 : std::max<std::int64_t>(0, best[window.front()]);
        best[i] = nums[i] + prev;
        answer = std::max(answer, best[i]);
        while (!window.empty() && best[window.back()] <= best[i]) window.pop_back();
        window.push_back(i);
    }
    return answer;
}

namespace detail {
struct PartitionLayer {
    const std::vector<std::int64_t>& prefix;
    const std::vector<std::int64_t>& prev;  // dp[g-1][*]
    std::vector<std::int64_t>& cur;         // dp[g][*]

    static constexpr std::int64_t kInf = INT64_MAX / 4;

    void solve(int lo, int hi, int opt_lo, int opt_hi) {
        if (lo > hi) return;
        int mid = lo + (hi - lo) / 2;
        std::int64_t best = kInf;
        int best_j = opt_lo;
        for (int j = opt_lo; j <= std::min(mid - 1, opt_hi); ++j) {
            if (prev[j] >= kInf) continue;
            std::int64_t s = prefix[mid] - prefix[j];
            std::int64_t c = prev[j] + s * s;
            if (c < best) best = c, best_j = j;
        }
        cur[mid] = best;
        solve(lo, mid - 1, opt_lo, best_j);
        solve(mid + 1, hi, best_j, opt_hi);
    }
};
}  // namespace detail

inline std::int64_t min_partition_cost(const std::vector<std::int64_t>& a, int k) {
    const int n = static_cast<int>(a.size());
    std::vector<std::int64_t> prefix(n + 1, 0);
    for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + a[i];
    constexpr std::int64_t kInf = detail::PartitionLayer::kInf;
    std::vector<std::int64_t> prev(n + 1, kInf), cur(n + 1, kInf);
    prev[0] = 0;  // zero elements in zero groups
    for (int g = 1; g <= k; ++g) {
        std::fill(cur.begin(), cur.end(), kInf);
        detail::PartitionLayer layer{prefix, prev, cur};
        layer.solve(g, n, g - 1, n - 1);
        std::swap(prev, cur);
    }
    return prev[n];
}

__extension__ using i128 = __int128;  // __extension__ keeps -Wpedantic quiet about the GCC/Clang type

inline std::int64_t frog_squared(const std::vector<std::int64_t>& h, std::int64_t C) {
    const int n = static_cast<int>(h.size());
    struct Line {
        std::int64_t m, b;
        std::int64_t at(std::int64_t x) const { return m * x + b; }
    };
    // l2 is useless if l1 and l3 together are never worse (slopes strictly decreasing).
    auto useless = [](const Line& l1, const Line& l2, const Line& l3) {
        return static_cast<i128>(l3.b - l1.b) * (l1.m - l2.m) <= static_cast<i128>(l2.b - l1.b) * (l1.m - l3.m);
    };
    std::deque<Line> hull;
    std::int64_t dp_i = 0;  // dp[0]
    hull.push_back({-2 * h[0], dp_i + h[0] * h[0]});
    for (int i = 1; i < n; ++i) {
        std::int64_t x = h[i];
        while (hull.size() >= 2 && hull[1].at(x) <= hull[0].at(x)) hull.pop_front();
        dp_i = hull[0].at(x) + x * x + C;
        Line nl{-2 * h[i], dp_i + h[i] * h[i]};
        while (hull.size() >= 2 && useless(hull[hull.size() - 2], hull.back(), nl)) hull.pop_back();
        hull.push_back(nl);
    }
    return dp_i;
}

}  // namespace dp
