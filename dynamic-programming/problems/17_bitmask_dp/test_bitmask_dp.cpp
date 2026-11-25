#include "bitmask_dp.hpp"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <random>
#include <set>
#include <vector>

#include "testing.hpp"

using dp::Matrix;

namespace {

Matrix random_matrix(std::mt19937_64& rng, int n, int max_v, bool symmetric) {
    Matrix m(n, std::vector<std::int64_t>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (i != j && (!symmetric || i < j)) m[i][j] = static_cast<std::int64_t>(rng() % (max_v + 1));
    if (symmetric)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < i; ++j) m[i][j] = m[j][i];
    return m;
}

std::int64_t tour_cost(const Matrix& d, const std::vector<int>& t) {
    std::int64_t c = 0;
    for (std::size_t i = 0; i + 1 < t.size(); ++i) c += d[t[i]][t[i + 1]];
    return c;
}

std::int64_t brute_tsp(const Matrix& d) {
    const int n = static_cast<int>(d.size());
    std::vector<int> perm(n - 1);
    std::iota(perm.begin(), perm.end(), 1);
    std::int64_t best = INT64_MAX;
    do {
        std::vector<int> t{0};
        t.insert(t.end(), perm.begin(), perm.end());
        t.push_back(0);
        best = std::min(best, tour_cost(d, t));
    } while (std::next_permutation(perm.begin(), perm.end()));
    return best;
}

std::int64_t brute_assign(const Matrix& c) {
    const int n = static_cast<int>(c.size());
    std::vector<int> p(n);
    std::iota(p.begin(), p.end(), 0);
    std::int64_t best = INT64_MAX;
    do {
        std::int64_t s = 0;
        for (int w = 0; w < n; ++w) s += c[w][p[w]];
        best = std::min(best, s);
    } while (std::next_permutation(p.begin(), p.end()));
    return best;
}

bool brute_partition(const std::vector<int>& nums, int k) {
    // Assign each item a group label 0..k-1 (k^n, small only).
    const int n = static_cast<int>(nums.size());
    std::vector<int> label(n, 0);
    while (true) {
        std::vector<int> sums(k, 0), counts(k, 0);
        for (int i = 0; i < n; ++i) sums[label[i]] += nums[i], ++counts[label[i]];
        bool ok = true;
        for (int g = 0; g < k; ++g) ok = ok && sums[g] == sums[0] && counts[g] > 0;
        if (ok) return true;
        int i = 0;
        while (i < n && label[i] == k - 1) label[i++] = 0;
        if (i == n) return false;
        ++label[i];
    }
}

}  // namespace

TEST(tsp_examples) {
    Matrix d{{0, 10, 15, 20}, {10, 0, 35, 25}, {15, 35, 0, 30}, {20, 25, 30, 0}};
    CHECK_EQ(dp::tsp_cost(d), 80);
    CHECK_EQ(dp::tsp_cost({{0}}), 0);
    CHECK_EQ(dp::tsp_cost({{0, 3}, {4, 0}}), 7);
    Matrix asym{{0, 1, 100}, {100, 0, 1}, {1, 100, 0}};  // 0->1->2->0 is cheap, reverse is not
    CHECK_EQ(dp::tsp_cost(asym), 3);
}

TEST(tsp_matches_brute_force) {
    std::mt19937_64 rng(48);
    for (int iter = 0; iter < 200; ++iter) {
        auto d = random_matrix(rng, 2 + static_cast<int>(rng() % 7), 100, iter % 2 == 0);
        CHECK_EQ(dp::tsp_cost(d), brute_tsp(d));
    }
}

TEST(tour_is_valid_and_optimal) {
    std::mt19937_64 rng(49);
    CHECK(dp::tsp_tour({{0}}) == (std::vector<int>{0, 0}));
    for (int iter = 0; iter < 200; ++iter) {
        const int n = 2 + static_cast<int>(rng() % 7);
        auto d = random_matrix(rng, n, 100, false);
        auto t = dp::tsp_tour(d);
        CHECK_EQ(static_cast<int>(t.size()), n + 1);
        CHECK_EQ(t.front(), 0);
        CHECK_EQ(t.back(), 0);
        CHECK_EQ(std::set<int>(t.begin(), t.end() - 1).size(), static_cast<std::size_t>(n));
        CHECK_EQ(tour_cost(d, t), brute_tsp(d));
    }
}

TEST(tsp_large) {
    // 16 cities on a circle: the optimal tour walks around it.
    const int n = 16;
    Matrix d(n, std::vector<std::int64_t>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            int gap = std::abs(i - j);
            d[i][j] = 1000 * std::min(gap, n - gap) + (i * 7 + j * 13) % 10;
        }
    std::int64_t c = dp::tsp_cost(d);
    CHECK_EQ(c, 16050);
    auto t = dp::tsp_tour(d);
    CHECK_EQ(tour_cost(d, t), c);
}

TEST(assignment_examples) {
    CHECK_EQ(dp::min_assignment_cost({{9, 2, 7, 8}, {6, 4, 3, 7}, {5, 8, 1, 8}, {7, 6, 9, 4}}), 13);
    CHECK_EQ(dp::min_assignment_cost({{5}}), 5);
    CHECK_EQ(dp::min_assignment_cost({{1, 2}, {2, 1}}), 2);
    CHECK_EQ(dp::min_assignment_cost({{1, 2}, {1, 5}}), 3);
}

TEST(assignment_matches_brute_force) {
    std::mt19937_64 rng(50);
    for (int iter = 0; iter < 200; ++iter) {
        auto c = random_matrix(rng, 1 + static_cast<int>(rng() % 7), 50, false);
        for (int i = 0; i < static_cast<int>(c.size()); ++i) c[i][i] = static_cast<std::int64_t>(rng() % 51);
        CHECK_EQ(dp::min_assignment_cost(c), brute_assign(c));
    }
}

TEST(assignment_large) {
    const int n = 20;
    Matrix c(n, std::vector<std::int64_t>(n));
    for (int w = 0; w < n; ++w)
        for (int j = 0; j < n; ++j) c[w][j] = (w * 7 + j * j * 13 + w * j * 5) % 1000 + 1;
    CHECK_EQ(dp::min_assignment_cost(c), 1800);
}

TEST(partition_examples) {
    CHECK(dp::can_partition_k({4, 3, 2, 3, 5, 2, 1}, 4));
    CHECK_FALSE(dp::can_partition_k({1, 2, 3, 4}, 3));
    CHECK(dp::can_partition_k({5}, 1));
    CHECK(dp::can_partition_k({2, 2, 2, 2, 3, 4, 5}, 4) == false);
    CHECK(dp::can_partition_k({1, 1, 1, 1}, 4));
    CHECK_FALSE(dp::can_partition_k({10, 1, 1}, 2));
}

TEST(partition_matches_brute_force) {
    std::mt19937_64 rng(51);
    for (int iter = 0; iter < 300; ++iter) {
        std::vector<int> nums(1 + rng() % 8);
        for (auto& x : nums) x = 1 + static_cast<int>(rng() % 6);
        int k = 1 + static_cast<int>(rng() % std::min<std::size_t>(nums.size(), 4));
        CHECK_EQ(dp::can_partition_k(nums, k), brute_partition(nums, k));
    }
}

TEST(partition_large) {
    std::vector<int> nums{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};  // sum 136
    CHECK(dp::can_partition_k(nums, 8));   // pairs summing to 17
    CHECK(dp::can_partition_k(nums, 4));   // 34 each
    CHECK_FALSE(dp::can_partition_k(nums, 16));
    std::vector<int> hard(16, 1000);
    hard[0] = 1003;
    hard[15] = 997;
    CHECK(dp::can_partition_k(hard, 8));
    hard[15] = 998;
    CHECK_FALSE(dp::can_partition_k(hard, 3));
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
