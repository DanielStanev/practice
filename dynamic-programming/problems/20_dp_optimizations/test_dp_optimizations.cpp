#include "dp_optimizations.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <random>
#include <vector>

#include "testing.hpp"

namespace {

using V = std::vector<std::int64_t>;

std::int64_t naive_constrained(const V& nums, int k) {
    const int n = static_cast<int>(nums.size());
    V best(n);
    std::int64_t answer = INT64_MIN;
    for (int i = 0; i < n; ++i) {
        std::int64_t m = 0;
        for (int j = std::max(0, i - k); j < i; ++j) m = std::max(m, best[j]);
        best[i] = nums[i] + m;
        answer = std::max(answer, best[i]);
    }
    return answer;
}

std::int64_t naive_partition(const V& a, int k) {
    const int n = static_cast<int>(a.size());
    const std::int64_t inf = INT64_MAX / 4;
    V prefix(n + 1, 0);
    for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + a[i];
    std::vector<V> d(k + 1, V(n + 1, inf));
    d[0][0] = 0;
    for (int g = 1; g <= k; ++g)
        for (int i = 1; i <= n; ++i)
            for (int j = 0; j < i; ++j)
                if (d[g - 1][j] < inf) {
                    std::int64_t s = prefix[i] - prefix[j];
                    d[g][i] = std::min(d[g][i], d[g - 1][j] + s * s);
                }
    return d[k][n];
}

std::int64_t naive_frog(const V& h, std::int64_t C) {
    const int n = static_cast<int>(h.size());
    V d(n, INT64_MAX / 4);
    d[0] = 0;
    for (int i = 1; i < n; ++i)
        for (int j = 0; j < i; ++j) d[i] = std::min(d[i], d[j] + (h[i] - h[j]) * (h[i] - h[j]) + C);
    return d[n - 1];
}

V increasing(std::mt19937_64& rng, int n, int max_step) {
    V h(n);
    std::int64_t x = 0;
    for (auto& v : h) v = (x += 1 + static_cast<std::int64_t>(rng() % max_step));
    return h;
}

template <typename F>
long long elapsed_ms(F&& f) {
    auto t0 = std::chrono::steady_clock::now();
    f();
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
}

}  // namespace

TEST(constrained_examples) {
    CHECK_EQ(dp::max_constrained_sum({10, 2, -10, 5, 20}, 2), 37);
    CHECK_EQ(dp::max_constrained_sum({-1, -2, -3}, 1), -1);
    CHECK_EQ(dp::max_constrained_sum({10, -2, -10, -5, 20}, 2), 23);
    CHECK_EQ(dp::max_constrained_sum({5}, 1), 5);
}

TEST(constrained_matches_naive) {
    std::mt19937_64 rng(60);
    for (int iter = 0; iter < 500; ++iter) {
        V nums(1 + rng() % 40);
        for (auto& x : nums) x = static_cast<std::int64_t>(rng() % 41) - 25;
        int k = 1 + static_cast<int>(rng() % nums.size());
        CHECK_EQ(dp::max_constrained_sum(nums, k), naive_constrained(nums, k));
    }
}

TEST(constrained_large) {
    std::mt19937_64 rng(61);
    V nums(300000);
    for (auto& x : nums) x = static_cast<std::int64_t>(rng() % 20001) - 10000;
    std::int64_t r = 0;
    auto ms = elapsed_ms([&] { r = dp::max_constrained_sum(nums, 150000); });
    std::int64_t positives = 0;
    for (auto x : nums) positives += std::max<std::int64_t>(0, x);
    CHECK_EQ(r, positives);  // with a huge window you just take every positive number
    V neg(300000, -5);
    for (std::size_t i = 0; i < neg.size(); i += 1000) neg[i] = 3;
    CHECK_EQ(dp::max_constrained_sum(neg, 999), 3);
    CHECK_EQ(dp::max_constrained_sum(neg, 1000), 300 * 3);
    CHECK(ms < 3000);
}

TEST(partition_examples) {
    CHECK_EQ(dp::min_partition_cost({1, 2, 3, 4}, 1), 100);
    CHECK_EQ(dp::min_partition_cost({1, 2, 3, 4}, 4), 30);
    CHECK_EQ(dp::min_partition_cost({1, 2, 3, 4}, 2), 52);  // {1,2,3} {4}: 36 + 16
    CHECK_EQ(dp::min_partition_cost({5, 0, 0, 5}, 2), 50);
    CHECK_EQ(dp::min_partition_cost({7}, 1), 49);
}

TEST(partition_matches_naive) {
    std::mt19937_64 rng(62);
    for (int iter = 0; iter < 300; ++iter) {
        V a(1 + rng() % 30);
        for (auto& x : a) x = static_cast<std::int64_t>(rng() % 20);
        int k = 1 + static_cast<int>(rng() % a.size());
        CHECK_EQ(dp::min_partition_cost(a, k), naive_partition(a, k));
    }
}

TEST(partition_matches_naive_medium) {
    std::mt19937_64 rng(63);
    V a(300);
    for (auto& x : a) x = static_cast<std::int64_t>(rng() % 10001);
    for (int k : {1, 2, 7, 50})
        CHECK_EQ(dp::min_partition_cost(a, k), naive_partition(a, k));
}

TEST(partition_large) {
    std::mt19937_64 rng(64);
    V a(20000);
    for (auto& x : a) x = static_cast<std::int64_t>(rng() % 10001);
    std::int64_t r = 0;
    auto ms = elapsed_ms([&] { r = dp::min_partition_cost(a, 50); });
    CHECK_EQ(r, 200716595341274LL);
    V ones(20000, 10000);
    CHECK_EQ(dp::min_partition_cost(ones, 50), 50LL * (400LL * 10000) * (400LL * 10000));
    CHECK(ms < 10000);
}

TEST(frog_examples) {
    CHECK_EQ(dp::frog_squared({1, 2, 3, 4, 5}, 6), 20);
    CHECK_EQ(dp::frog_squared({1, 1000000}, 1'000'000'000'000LL), 999999LL * 999999LL + 1'000'000'000'000LL);
    CHECK_EQ(dp::frog_squared({1, 3, 4, 5, 10, 11, 12, 13}, 5), 62);
    CHECK_EQ(dp::frog_squared({3}, 7), 0);
}

TEST(frog_matches_naive) {
    std::mt19937_64 rng(65);
    for (int iter = 0; iter < 500; ++iter) {
        V h = increasing(rng, 1 + static_cast<int>(rng() % 40), 30);
        std::int64_t C = 1 + static_cast<std::int64_t>(rng() % 500);
        CHECK_EQ(dp::frog_squared(h, C), naive_frog(h, C));
    }
}

TEST(frog_matches_naive_medium) {
    std::mt19937_64 rng(66);
    for (std::int64_t C : {1LL, 1000LL, 1'000'000'000'000LL}) {
        V h = increasing(rng, 3000, 300);
        CHECK_EQ(dp::frog_squared(h, C), naive_frog(h, C));
    }
}

TEST(frog_large) {
    std::mt19937_64 rng(67);
    V h = increasing(rng, 300000, 5);  // average step 3, so heights stay under 10^6
    std::int64_t r = 0;
    auto ms = elapsed_ms([&] { r = dp::frog_squared(h, 1'000'000); });
    CHECK_EQ(r, 1800640906LL);
    CHECK(ms < 3000);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
