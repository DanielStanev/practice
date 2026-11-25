#include "grid_paths.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "testing.hpp"

namespace {

using Grid = std::vector<std::string>;
using Costs = std::vector<std::vector<std::int64_t>>;

// Brute force: enumerate paths recursively.
std::int64_t brute_paths(const Grid& g, int r, int c) {
    const int R = static_cast<int>(g.size()), C = static_cast<int>(g[0].size());
    if (r >= R || c >= C || g[r][c] == '#') return 0;
    if (r == R - 1 && c == C - 1) return 1;
    return brute_paths(g, r + 1, c) + brute_paths(g, r, c + 1);
}

std::int64_t brute_min(const Costs& g, int r, int c) {
    const int R = static_cast<int>(g.size()), C = static_cast<int>(g[0].size());
    if (r == R - 1 && c == C - 1) return g[r][c];
    std::int64_t best = INT64_MAX;
    if (r + 1 < R) best = std::min(best, brute_min(g, r + 1, c));
    if (c + 1 < C) best = std::min(best, brute_min(g, r, c + 1));
    return g[r][c] + best;
}

int brute_square(const Grid& g) {
    const int R = static_cast<int>(g.size()), C = static_cast<int>(g[0].size());
    int best = 0;
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            for (int k = 1; r + k <= R && c + k <= C; ++k) {
                bool ok = true;
                for (int i = 0; i < k && ok; ++i)
                    for (int j = 0; j < k && ok; ++j) ok = g[r + i][c + j] == '1';
                if (ok) best = std::max(best, k);
            }
    return best;
}

}  // namespace

TEST(unique_paths_examples) {
    CHECK_EQ(dp::unique_paths(1, 1), 1ULL);
    CHECK_EQ(dp::unique_paths(3, 7), 28ULL);
    CHECK_EQ(dp::unique_paths(3, 2), 3ULL);
    CHECK_EQ(dp::unique_paths(3, 4), 10ULL);
    CHECK_EQ(dp::unique_paths(1, 30), 1ULL);
    CHECK_EQ(dp::unique_paths(30, 30), 30067266499541040ULL);
}

TEST(obstacles_examples) {
    CHECK_EQ(dp::unique_paths_with_obstacles({"...", ".#.", "..."}), 2);
    CHECK_EQ(dp::unique_paths_with_obstacles({".#", ".."}), 1);
    CHECK_EQ(dp::unique_paths_with_obstacles({"#.", ".."}), 0);
    CHECK_EQ(dp::unique_paths_with_obstacles({"..", ".#"}), 0);
    CHECK_EQ(dp::unique_paths_with_obstacles({"."}), 1);
    CHECK_EQ(dp::unique_paths_with_obstacles({".#..", "...."}), 1);
}

TEST(obstacles_match_brute_force) {
    std::mt19937_64 rng(13);
    for (int iter = 0; iter < 500; ++iter) {
        Grid g(1 + rng() % 7, std::string(1 + rng() % 7, '.'));
        for (auto& row : g)
            for (auto& ch : row) ch = rng() % 5 == 0 ? '#' : '.';
        CHECK_EQ(dp::unique_paths_with_obstacles(g), brute_paths(g, 0, 0));
    }
}

TEST(obstacles_large_uses_modulo) {
    Grid g(1000, std::string(1000, '.'));
    CHECK_EQ(dp::unique_paths_with_obstacles(g), 965601742);
    for (int i = 7; i < 1000; i += 7) g[i][(i * 31) % 1000] = '#';
    CHECK_EQ(dp::unique_paths_with_obstacles(g), 935156062);
}

TEST(min_path_sum_examples) {
    CHECK_EQ(dp::min_path_sum({{1, 3, 1}, {1, 5, 1}, {4, 2, 1}}), 7);
    CHECK_EQ(dp::min_path_sum({{1, 2, 3}, {4, 5, 6}}), 12);
    CHECK_EQ(dp::min_path_sum({{5}}), 5);
}

TEST(min_path_sum_matches_brute_force) {
    std::mt19937_64 rng(14);
    for (int iter = 0; iter < 500; ++iter) {
        Costs g(1 + rng() % 7, std::vector<std::int64_t>(1 + rng() % 7));
        for (auto& row : g)
            for (auto& x : row) x = static_cast<std::int64_t>(rng() % 10);
        CHECK_EQ(dp::min_path_sum(g), brute_min(g, 0, 0));
    }
}

TEST(min_path_sum_large) {
    Costs g(1000, std::vector<std::int64_t>(1000, 1'000'000'000));
    CHECK_EQ(dp::min_path_sum(g), 1999LL * 1'000'000'000LL);
}

TEST(square_examples) {
    CHECK_EQ(dp::largest_square({"10100", "10111", "11111", "10010"}), 2);
    CHECK_EQ(dp::largest_square({"01", "10"}), 1);
    CHECK_EQ(dp::largest_square({"0"}), 0);
    CHECK_EQ(dp::largest_square({"111", "111", "111"}), 3);
    CHECK_EQ(dp::largest_square({"1111", "1101", "1111"}), 2);
}

TEST(square_matches_brute_force) {
    std::mt19937_64 rng(15);
    for (int iter = 0; iter < 500; ++iter) {
        Grid g(1 + rng() % 8, std::string(1 + rng() % 8, '0'));
        for (auto& row : g)
            for (auto& ch : row) ch = rng() % 4 == 0 ? '0' : '1';
        CHECK_EQ(dp::largest_square(g), brute_square(g));
    }
}

TEST(square_large) {
    Grid g(1000, std::string(1000, '1'));
    CHECK_EQ(dp::largest_square(g), 1000);
    g[500][500] = '0';
    CHECK_EQ(dp::largest_square(g), 500);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
