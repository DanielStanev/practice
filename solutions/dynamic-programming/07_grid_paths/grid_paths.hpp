// Problem 07: grid paths. Reference solution (one-row tables).
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace dp {

inline constexpr std::int64_t kMod = 1'000'000'007;

inline std::uint64_t unique_paths(int rows, int cols) {
    std::vector<std::uint64_t> row(cols, 1);  // first row: one path to each cell
    for (int r = 1; r < rows; ++r)
        for (int c = 1; c < cols; ++c) row[c] += row[c - 1];  // above + left
    return row[cols - 1];
}

inline std::int64_t unique_paths_with_obstacles(const std::vector<std::string>& grid) {
    const std::size_t cols = grid[0].size();
    std::vector<std::int64_t> row(cols, 0);
    row[0] = 1;  // virtual "path into" the start; zeroed below if blocked
    for (const auto& line : grid)
        for (std::size_t c = 0; c < cols; ++c) {
            if (line[c] == '#') row[c] = 0;
            else if (c > 0) row[c] = (row[c] + row[c - 1]) % kMod;
        }
    return row[cols - 1];
}

inline std::int64_t min_path_sum(const std::vector<std::vector<std::int64_t>>& cost) {
    constexpr std::int64_t kInf = INT64_MAX / 4;
    const std::size_t cols = cost[0].size();
    std::vector<std::int64_t> row(cols, kInf);
    row[0] = 0;
    for (const auto& line : cost)
        for (std::size_t c = 0; c < cols; ++c) {
            std::int64_t from = row[c];                       // from above
            if (c > 0) from = std::min(from, row[c - 1]);     // from left
            row[c] = from + line[c];
        }
    return row[cols - 1];
}

inline int largest_square(const std::vector<std::string>& grid) {
    const std::size_t cols = grid[0].size();
    std::vector<int> row(cols + 1, 0);  // row[c + 1] holds column c; row[0] is a zero border
    int best = 0;
    for (const auto& line : grid) {
        int diag = 0;  // previous row's value at column c - 1
        for (std::size_t c = 0; c < cols; ++c) {
            int above = row[c + 1];
            row[c + 1] = line[c] == '1' ? 1 + std::min({above, row[c], diag}) : 0;
            diag = above;
            best = std::max(best, row[c + 1]);
        }
    }
    return best;
}

}  // namespace dp
