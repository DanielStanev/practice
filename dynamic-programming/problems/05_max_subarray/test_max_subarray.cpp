#include "max_subarray.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include "testing.hpp"

namespace {

using V = std::vector<std::int64_t>;

std::int64_t brute_sum(const V& a) {
    std::int64_t best = a[0];
    for (std::size_t i = 0; i < a.size(); ++i) {
        std::int64_t s = 0;
        for (std::size_t j = i; j < a.size(); ++j) best = std::max(best, s += a[j]);
    }
    return best;
}

std::int64_t brute_product(const V& a) {
    std::int64_t best = a[0];
    for (std::size_t i = 0; i < a.size(); ++i) {
        std::int64_t p = 1;
        for (std::size_t j = i; j < a.size(); ++j) best = std::max(best, p *= a[j]);
    }
    return best;
}

std::int64_t brute_trade(const V& p) {
    std::int64_t best = 0;
    for (std::size_t i = 0; i < p.size(); ++i)
        for (std::size_t j = i + 1; j < p.size(); ++j) best = std::max(best, p[j] - p[i]);
    return best;
}

}  // namespace

TEST(sum_examples) {
    CHECK_EQ(dp::max_subarray_sum({-2, 1, -3, 4, -1, 2, 1, -5, 4}), 6);
    CHECK_EQ(dp::max_subarray_sum({1}), 1);
    CHECK_EQ(dp::max_subarray_sum({5, 4, -1, 7, 8}), 23);
    CHECK_EQ(dp::max_subarray_sum({-3, -1, -2}), -1);  // must be non-empty
}

TEST(sum_matches_brute_force) {
    std::mt19937_64 rng(7);
    for (int iter = 0; iter < 1000; ++iter) {
        V a(1 + rng() % 25);
        for (auto& x : a) x = static_cast<std::int64_t>(rng() % 41) - 20;
        CHECK_EQ(dp::max_subarray_sum(a), brute_sum(a));
    }
}

TEST(sum_large) {
    V a(300000, -1);
    a[150000] = 1'000'000'000'000LL;
    a[150001] = 1'000'000'000'000LL;
    CHECK_EQ(dp::max_subarray_sum(a), 2'000'000'000'000LL);
}

TEST(range_examples) {
    auto r = dp::max_subarray_range({-2, 1, -3, 4, -1, 2, 1, -5, 4});
    CHECK_EQ(r.first, 3u);
    CHECK_EQ(r.second, 7u);
    auto r2 = dp::max_subarray_range({-3, -1, -2});
    CHECK_EQ(r2.first, 1u);
    CHECK_EQ(r2.second, 2u);
}

TEST(range_is_valid_and_optimal) {
    std::mt19937_64 rng(8);
    for (int iter = 0; iter < 1000; ++iter) {
        V a(1 + rng() % 25);
        for (auto& x : a) x = static_cast<std::int64_t>(rng() % 41) - 20;
        auto [b, e] = dp::max_subarray_range(a);
        CHECK(b < e);
        CHECK(e <= a.size());
        std::int64_t s = 0;
        for (std::size_t i = b; i < e; ++i) s += a[i];
        CHECK_EQ(s, brute_sum(a));
    }
}

TEST(trade_examples) {
    CHECK_EQ(dp::max_profit_one_trade({7, 1, 5, 3, 6, 4}), 5);
    CHECK_EQ(dp::max_profit_one_trade({7, 6, 4, 3, 1}), 0);
    CHECK_EQ(dp::max_profit_one_trade({}), 0);
    CHECK_EQ(dp::max_profit_one_trade({3}), 0);
    CHECK_EQ(dp::max_profit_one_trade({2, 4, 1, 3}), 2);
    CHECK_EQ(dp::max_profit_one_trade({3, 8, 1, 7}), 6);  // later minimum wins
}

TEST(trade_matches_brute_force) {
    std::mt19937_64 rng(9);
    for (int iter = 0; iter < 1000; ++iter) {
        V p(rng() % 25);
        for (auto& x : p) x = 100 + static_cast<std::int64_t>(rng() % 50);
        CHECK_EQ(dp::max_profit_one_trade(p), brute_trade(p));
    }
}

TEST(product_examples) {
    CHECK_EQ(dp::max_product_subarray({2, 3, -2, 4}), 6);
    CHECK_EQ(dp::max_product_subarray({-2, 0, -1}), 0);
    CHECK_EQ(dp::max_product_subarray({-2, 3, -4}), 24);
    CHECK_EQ(dp::max_product_subarray({-2}), -2);
    CHECK_EQ(dp::max_product_subarray({0, 2}), 2);
    CHECK_EQ(dp::max_product_subarray({-1, -1, -1}), 1);
}

TEST(product_matches_brute_force) {
    std::mt19937_64 rng(10);
    for (int iter = 0; iter < 2000; ++iter) {
        V a(1 + rng() % 18);
        for (auto& x : a) x = static_cast<std::int64_t>(rng() % 9) - 4;
        CHECK_EQ(dp::max_product_subarray(a), brute_product(a));
    }
}

TEST(product_large) {
    V a(300000, 1);
    for (int i = 0; i < 30; ++i) a[1000 + i] = (i % 2 == 0) ? -3 : 3;  // 15 negatives -> odd
    a[5000] = -1;                                                        // makes the count even
    std::int64_t p = 1;
    for (int i = 0; i < 30; ++i) p *= 3;
    CHECK_EQ(dp::max_product_subarray(a), p);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
