// Problem 12: edit distance. Reference solution.
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace dp {

enum class Op { Keep, Replace, Insert, Delete };

struct Edit {
    Op op;
    char ch;
};

inline std::int64_t edit_distance_weighted(std::string_view a, std::string_view b, std::int64_t insert_cost,
                                           std::int64_t delete_cost, std::int64_t replace_cost) {
    const std::size_t m = b.size();
    std::vector<std::int64_t> row(m + 1);
    for (std::size_t j = 0; j <= m; ++j) row[j] = static_cast<std::int64_t>(j) * insert_cost;
    for (std::size_t i = 1; i <= a.size(); ++i) {
        std::int64_t diag = row[0];
        row[0] = static_cast<std::int64_t>(i) * delete_cost;
        for (std::size_t j = 1; j <= m; ++j) {
            std::int64_t up = row[j];
            std::int64_t best = diag + (a[i - 1] == b[j - 1] ? 0 : replace_cost);
            best = std::min({best, up + delete_cost, row[j - 1] + insert_cost});
            row[j] = best;
            diag = up;
        }
    }
    return row[m];
}

inline int edit_distance(std::string_view a, std::string_view b) {
    if (a.size() < b.size()) std::swap(a, b);  // distance is symmetric; keep the row short
    return static_cast<int>(edit_distance_weighted(a, b, 1, 1, 1));
}

inline std::vector<Edit> edit_script(std::string_view a, std::string_view b) {
    const std::size_t n = a.size(), m = b.size(), W = m + 1;
    std::vector<int> E((n + 1) * W);
    auto at = [&](std::size_t i, std::size_t j) -> int& { return E[i * W + j]; };
    for (std::size_t i = 0; i <= n; ++i) at(i, 0) = static_cast<int>(i);
    for (std::size_t j = 0; j <= m; ++j) at(0, j) = static_cast<int>(j);
    for (std::size_t i = 1; i <= n; ++i)
        for (std::size_t j = 1; j <= m; ++j)
            at(i, j) = std::min({at(i - 1, j - 1) + (a[i - 1] == b[j - 1] ? 0 : 1), at(i - 1, j) + 1, at(i, j - 1) + 1});
    std::vector<Edit> script;
    std::size_t i = n, j = m;
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && a[i - 1] == b[j - 1] && at(i, j) == at(i - 1, j - 1)) {
            script.push_back({Op::Keep, a[i - 1]});
            --i, --j;
        } else if (i > 0 && j > 0 && at(i, j) == at(i - 1, j - 1) + 1) {
            script.push_back({Op::Replace, b[j - 1]});
            --i, --j;
        } else if (i > 0 && at(i, j) == at(i - 1, j) + 1) {
            script.push_back({Op::Delete, 0});
            --i;
        } else {
            script.push_back({Op::Insert, b[j - 1]});
            --j;
        }
    }
    std::reverse(script.begin(), script.end());
    return script;
}

}  // namespace dp
