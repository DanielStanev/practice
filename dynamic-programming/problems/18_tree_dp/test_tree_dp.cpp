#include "tree_dp.hpp"

#include <algorithm>
#include <cstdint>
#include <queue>
#include <random>
#include <vector>

#include "testing.hpp"

using dp::Edges;

namespace {

Edges random_tree(std::mt19937_64& rng, int n) {
    Edges e;
    std::vector<int> label(n);
    for (int i = 0; i < n; ++i) label[i] = i;
    std::shuffle(label.begin(), label.end(), rng);
    for (int i = 1; i < n; ++i) e.push_back({label[i], label[rng() % i]});
    return e;
}

std::vector<int> bfs(int n, const Edges& e, int src) {
    std::vector<std::vector<int>> adj(n);
    for (auto [a, b] : e) adj[a].push_back(b), adj[b].push_back(a);
    std::vector<int> d(n, -1);
    std::queue<int> q;
    d[src] = 0;
    q.push(src);
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        for (int c : adj[v])
            if (d[c] < 0) d[c] = d[v] + 1, q.push(c);
    }
    return d;
}

std::int64_t brute_mis(int n, const Edges& e, const std::vector<std::int64_t>& w) {
    std::int64_t best = 0;
    for (int mask = 0; mask < (1 << n); ++mask) {
        bool ok = true;
        for (auto [a, b] : e) ok = ok && !((mask >> a & 1) && (mask >> b & 1));
        if (!ok) continue;
        std::int64_t s = 0;
        for (int i = 0; i < n; ++i)
            if (mask >> i & 1) s += w[i];
        best = std::max(best, s);
    }
    return best;
}

Edges path(int n) {
    Edges e;
    for (int i = 0; i + 1 < n; ++i) e.push_back({i, i + 1});
    return e;
}

}  // namespace

TEST(mis_examples) {
    CHECK_EQ(dp::max_weight_independent_set(1, {}, {5}), 5);
    CHECK_EQ(dp::max_weight_independent_set(3, {{0, 1}, {1, 2}}, {3, 5, 3}), 6);
    CHECK_EQ(dp::max_weight_independent_set(3, {{0, 1}, {1, 2}}, {3, 7, 3}), 7);
    // star: center 0 with 4 leaves
    CHECK_EQ(dp::max_weight_independent_set(5, {{0, 1}, {0, 2}, {0, 3}, {0, 4}}, {10, 3, 3, 3, 3}), 12);
}

TEST(mis_matches_brute_force) {
    std::mt19937_64 rng(52);
    for (int iter = 0; iter < 300; ++iter) {
        int n = 1 + static_cast<int>(rng() % 14);
        auto e = random_tree(rng, n);
        std::vector<std::int64_t> w(n);
        for (auto& x : w) x = static_cast<std::int64_t>(rng() % 20);
        CHECK_EQ(dp::max_weight_independent_set(n, e, w), brute_mis(n, e, w));
    }
}

TEST(diameter_examples) {
    CHECK_EQ(dp::tree_diameter(1, {}), 0);
    CHECK_EQ(dp::tree_diameter(2, {{0, 1}}), 1);
    CHECK_EQ(dp::tree_diameter(5, {{0, 1}, {0, 2}, {0, 3}, {0, 4}}), 2);
    CHECK_EQ(dp::tree_diameter(6, {{0, 1}, {1, 2}, {2, 3}, {1, 4}, {4, 5}}), 4);
}

TEST(diameter_matches_double_bfs) {
    std::mt19937_64 rng(53);
    for (int iter = 0; iter < 300; ++iter) {
        int n = 1 + static_cast<int>(rng() % 60);
        auto e = random_tree(rng, n);
        auto d0 = bfs(n, e, 0);
        int far = static_cast<int>(std::max_element(d0.begin(), d0.end()) - d0.begin());
        auto d1 = bfs(n, e, far);
        CHECK_EQ(dp::tree_diameter(n, e), *std::max_element(d1.begin(), d1.end()));
    }
}

TEST(distances_examples) {
    auto a = dp::sum_of_distances(6, {{0, 1}, {0, 2}, {2, 3}, {2, 4}, {2, 5}});
    CHECK(a == (std::vector<std::int64_t>{8, 12, 6, 10, 10, 10}));
    CHECK(dp::sum_of_distances(1, {}) == std::vector<std::int64_t>{0});
    CHECK(dp::sum_of_distances(2, {{1, 0}}) == (std::vector<std::int64_t>{1, 1}));
}

TEST(distances_match_bfs_from_every_node) {
    std::mt19937_64 rng(54);
    for (int iter = 0; iter < 200; ++iter) {
        int n = 1 + static_cast<int>(rng() % 50);
        auto e = random_tree(rng, n);
        auto ans = dp::sum_of_distances(n, e);
        CHECK_EQ(static_cast<int>(ans.size()), n);
        for (int v = 0; v < n; ++v) {
            auto d = bfs(n, e, v);
            std::int64_t s = 0;
            for (int x : d) s += x;
            CHECK_EQ(ans[v], s);
        }
    }
}

TEST(deep_path_mis) {
    const int n = 200000;
    std::vector<std::int64_t> w(n, 1'000'000'000);
    CHECK_EQ(dp::max_weight_independent_set(n, path(n), w), 100000LL * 1'000'000'000LL);
}

TEST(deep_path_diameter) {
    const int n = 200000;
    CHECK_EQ(dp::tree_diameter(n, path(n)), n - 1);
}

TEST(deep_path_distances) {
    const std::int64_t n = 200000;
    auto ans = dp::sum_of_distances(static_cast<int>(n), path(static_cast<int>(n)));
    CHECK_EQ(ans[0], n * (n - 1) / 2);
    CHECK_EQ(ans[n - 1], n * (n - 1) / 2);
    std::int64_t mid = n / 2;  // distances to the left: 1..mid, to the right: 1..(n-1-mid)
    CHECK_EQ(ans[mid], mid * (mid + 1) / 2 + (n - 1 - mid) * (n - mid) / 2);
}

TEST(large_random_tree) {
    std::mt19937_64 rng(55);
    const int n = 200000;
    auto e = random_tree(rng, n);
    auto ans = dp::sum_of_distances(n, e);
    for (int v : {0, 1, 12345, n - 1}) {
        auto d = bfs(n, e, v);
        std::int64_t s = 0;
        for (int x : d) s += x;
        CHECK_EQ(ans[v], s);
    }
    auto d0 = bfs(n, e, 0);
    int far = static_cast<int>(std::max_element(d0.begin(), d0.end()) - d0.begin());
    auto d1 = bfs(n, e, far);
    CHECK_EQ(dp::tree_diameter(n, e), *std::max_element(d1.begin(), d1.end()));
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
