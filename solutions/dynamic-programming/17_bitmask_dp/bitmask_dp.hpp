// Problem 17: bitmask DP. Reference solution.
#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace dp {

using Matrix = std::vector<std::vector<std::int64_t>>;

namespace detail {
inline constexpr std::int64_t kInf = INT64_MAX / 4;

// Fills best[mask * n + last] and parent; returns {cost, last city before returning to 0}.
inline std::pair<std::int64_t, int> held_karp(const Matrix& dist, std::vector<std::int64_t>& best,
                                              std::vector<std::int8_t>& parent) {
    const int n = static_cast<int>(dist.size());
    const std::uint32_t full = (1u << n) - 1;
    best.assign(static_cast<std::size_t>(full + 1) * n, kInf);
    parent.assign(best.size(), -1);
    best[1 * n + 0] = 0;
    for (std::uint32_t mask = 1; mask <= full; mask += 2) {  // only masks containing city 0
        for (int last = 0; last < n; ++last) {
            std::int64_t cur = best[mask * n + last];
            if (cur >= kInf) continue;
            for (int next = 1; next < n; ++next) {
                if (mask >> next & 1) continue;
                std::uint32_t nm = mask | (1u << next);
                std::int64_t cand = cur + dist[last][next];
                if (cand < best[nm * n + next]) {
                    best[nm * n + next] = cand;
                    parent[nm * n + next] = static_cast<std::int8_t>(last);
                }
            }
        }
    }
    std::int64_t answer = kInf;
    int end = 0;
    for (int last = 0; last < n; ++last) {
        if (n > 1 && last == 0) continue;
        std::int64_t c = best[full * n + last] + dist[last][0];
        if (c < answer) answer = c, end = last;
    }
    return {answer, end};
}
}  // namespace detail

inline std::int64_t tsp_cost(const Matrix& dist) {
    std::vector<std::int64_t> best;
    std::vector<std::int8_t> parent;
    return detail::held_karp(dist, best, parent).first;
}

inline std::vector<int> tsp_tour(const Matrix& dist) {
    const int n = static_cast<int>(dist.size());
    std::vector<std::int64_t> best;
    std::vector<std::int8_t> parent;
    auto [cost, last] = detail::held_karp(dist, best, parent);
    (void)cost;
    std::vector<int> tour{0};
    std::uint32_t mask = (1u << n) - 1;
    while (last != 0) {
        tour.push_back(last);
        int prev = parent[mask * n + last];
        mask &= ~(1u << last);
        last = prev;
    }
    tour.push_back(0);
    std::reverse(tour.begin(), tour.end());
    return tour;
}

inline std::int64_t min_assignment_cost(const Matrix& cost) {
    const int n = static_cast<int>(cost.size());
    const std::uint32_t full = (1u << n) - 1;
    std::vector<std::int64_t> best(full + 1, detail::kInf);
    best[0] = 0;
    for (std::uint32_t mask = 0; mask < full; ++mask) {
        if (best[mask] >= detail::kInf) continue;
        int worker = std::popcount(mask);
        for (int j = 0; j < n; ++j)
            if (!(mask >> j & 1)) {
                std::uint32_t nm = mask | (1u << j);
                best[nm] = std::min(best[nm], best[mask] + cost[worker][j]);
            }
    }
    return best[full];
}

inline bool can_partition_k(const std::vector<int>& nums, int k) {
    const int n = static_cast<int>(nums.size());
    int total = std::accumulate(nums.begin(), nums.end(), 0);
    if (total % k != 0) return false;
    const int target = total / k;
    if (*std::max_element(nums.begin(), nums.end()) > target) return false;
    const std::uint32_t full = (1u << n) - 1;
    std::vector<int> fill(full + 1, -1);  // fill level of the current group, -1 = unreachable
    fill[0] = 0;
    for (std::uint32_t mask = 0; mask < full; ++mask) {
        if (fill[mask] < 0) continue;
        for (int i = 0; i < n; ++i) {
            if (mask >> i & 1) continue;
            if (fill[mask] + nums[i] > target) continue;
            fill[mask | (1u << i)] = (fill[mask] + nums[i]) % target;
        }
    }
    return fill[full] == 0;
}

}  // namespace dp
