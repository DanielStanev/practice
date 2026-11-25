#include "min_cost_stairs.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include "testing.hpp"

namespace {

using V = std::vector<std::int64_t>;

// Brute force over every route: recursion without memo, small inputs only.
std::int64_t brute_stairs_from(const V& c, int pos) {
    const int n = static_cast<int>(c.size());
    if (pos >= n) return pos == n ? 0 : INT64_MAX / 4;
    return c[pos] + std::min(brute_stairs_from(c, pos + 1), brute_stairs_from(c, pos + 2));
}
std::int64_t brute_stairs(const V& c) { return std::min(brute_stairs_from(c, 0), brute_stairs_from(c, 1)); }

std::int64_t brute_frog(const V& h, int k, int pos) {
    const int n = static_cast<int>(h.size());
    if (pos == n - 1) return 0;
    std::int64_t best = INT64_MAX / 4;
    for (int j = pos + 1; j <= std::min(n - 1, pos + k); ++j) {
        std::int64_t d = h[pos] > h[j] ? h[pos] - h[j] : h[j] - h[pos];
        best = std::min(best, d + brute_frog(h, k, j));
    }
    return best;
}

// Checks that `path` is a legal route and returns its cost.
std::int64_t route_cost(const V& c, const std::vector<int>& path) {
    const int n = static_cast<int>(c.size());
    CHECK(!path.empty());
    CHECK(path.front() == 0 || path.front() == 1);
    std::int64_t total = 0;
    for (std::size_t i = 0; i < path.size(); ++i) {
        CHECK(path[i] >= 0 && path[i] < n);
        if (i > 0) CHECK(path[i] - path[i - 1] == 1 || path[i] - path[i - 1] == 2);
        total += c[path[i]];
    }
    CHECK(n - path.back() == 1 || n - path.back() == 2);
    return total;
}

}  // namespace

TEST(climb_examples) {
    CHECK_EQ(dp::min_cost_climb({10, 15, 20}), 15);
    CHECK_EQ(dp::min_cost_climb({1, 100, 1, 1, 1, 100, 1, 1, 100, 1}), 6);
    CHECK_EQ(dp::min_cost_climb({0, 0}), 0);
    CHECK_EQ(dp::min_cost_climb({5, 3}), 3);
}

TEST(climb_matches_brute_force) {
    std::mt19937_64 rng(1);
    for (int iter = 0; iter < 400; ++iter) {
        V c(2 + rng() % 16);
        for (auto& x : c) x = static_cast<std::int64_t>(rng() % 50);
        CHECK_EQ(dp::min_cost_climb(c), brute_stairs(c));
    }
}

TEST(climb_large_needs_int64) {
    V c(200000, 1'000'000'000);
    CHECK_EQ(dp::min_cost_climb(c), 100000LL * 1'000'000'000LL);
}

TEST(path_examples) {
    CHECK(dp::min_cost_path({10, 15, 20}) == std::vector<int>{1});
    CHECK(dp::min_cost_path({1, 100, 1, 1, 1, 100, 1, 1, 100, 1}) == (std::vector<int>{0, 2, 4, 6, 7, 9}));
}

TEST(path_is_valid_and_optimal) {
    std::mt19937_64 rng(2);
    for (int iter = 0; iter < 400; ++iter) {
        V c(2 + rng() % 30);
        for (auto& x : c) x = static_cast<std::int64_t>(rng() % 20);
        auto path = dp::min_cost_path(c);
        CHECK_EQ(route_cost(c, path), dp::min_cost_climb(c));
    }
}

TEST(frog_examples) {
    CHECK_EQ(dp::frog_min_energy({10, 30, 40, 20}, 1), 50);
    CHECK_EQ(dp::frog_min_energy({10, 30, 40, 20}, 2), 30);
    CHECK_EQ(dp::frog_min_energy({10, 30, 40, 50, 20}, 3), 30);
    CHECK_EQ(dp::frog_min_energy({10, 20, 10}, 1), 20);
    CHECK_EQ(dp::frog_min_energy({10, 10}, 100), 0);
    CHECK_EQ(dp::frog_min_energy({42}, 5), 0);
    CHECK_EQ(dp::frog_min_energy({40, 10, 20, 70, 80, 10, 20, 70, 80, 60}, 4), 40);
}

TEST(frog_matches_brute_force) {
    std::mt19937_64 rng(3);
    for (int iter = 0; iter < 300; ++iter) {
        V h(1 + rng() % 12);
        for (auto& x : h) x = 1 + static_cast<std::int64_t>(rng() % 100);
        int k = 1 + static_cast<int>(rng() % 4);
        CHECK_EQ(dp::frog_min_energy(h, k), brute_frog(h, k, 0));
    }
}

TEST(frog_large) {
    V h(100000);
    for (std::size_t i = 0; i < h.size(); ++i) h[i] = (i % 2 == 0) ? 1 : 1'000'000'000;
    // With k >= 2 the frog hops on even stones only; the last stone is odd, so one big jump is unavoidable.
    CHECK_EQ(dp::frog_min_energy(h, 100), 999'999'999);
    CHECK_EQ(dp::frog_min_energy(h, 1), 99999LL * 999'999'999LL);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
