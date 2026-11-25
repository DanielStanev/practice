// Problem 03: min-cost stairs and the frog. Reference solution.
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

namespace detail {
// best[k] and parent[k] for k in [0, n]; index n is the free "top".
inline void stairs_table(const std::vector<std::int64_t>& cost, std::vector<std::int64_t>& best,
                         std::vector<int>& parent) {
    const int n = static_cast<int>(cost.size());
    best.assign(n + 1, 0);
    parent.assign(n + 1, -1);
    best[0] = cost[0];
    best[1] = cost[1];
    for (int k = 2; k <= n; ++k) {
        std::int64_t here = k < n ? cost[k] : 0;
        if (best[k - 1] <= best[k - 2]) {
            best[k] = here + best[k - 1];
            parent[k] = k - 1;
        } else {
            best[k] = here + best[k - 2];
            parent[k] = k - 2;
        }
    }
}
}  // namespace detail

inline std::int64_t min_cost_climb(const std::vector<std::int64_t>& cost) {
    std::vector<std::int64_t> best;
    std::vector<int> parent;
    detail::stairs_table(cost, best, parent);
    return best[cost.size()];
}

inline std::vector<int> min_cost_path(const std::vector<std::int64_t>& cost) {
    std::vector<std::int64_t> best;
    std::vector<int> parent;
    detail::stairs_table(cost, best, parent);
    std::vector<int> path;
    for (int k = parent[cost.size()]; k != -1; k = parent[k]) path.push_back(k);
    std::reverse(path.begin(), path.end());
    return path;
}

inline std::int64_t frog_min_energy(const std::vector<std::int64_t>& h, int k) {
    constexpr std::int64_t kInf = INT64_MAX / 4;
    const int n = static_cast<int>(h.size());
    std::vector<std::int64_t> best(n, kInf);
    best[0] = 0;
    for (int j = 1; j < n; ++j)
        for (int i = std::max(0, j - k); i < j; ++i) {
            std::int64_t d = h[i] > h[j] ? h[i] - h[j] : h[j] - h[i];
            best[j] = std::min(best[j], best[i] + d);
        }
    return best[n - 1];
}

}  // namespace dp
