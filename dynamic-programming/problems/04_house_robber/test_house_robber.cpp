#include "house_robber.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <set>
#include <vector>

#include "testing.hpp"

namespace {

using V = std::vector<std::int64_t>;

// Try every subset (n <= 16).
std::int64_t brute(const V& m, bool circle) {
    const int n = static_cast<int>(m.size());
    std::int64_t best = 0;
    for (int mask = 0; mask < (1 << n); ++mask) {
        if (mask & (mask >> 1)) continue;
        if (circle && n > 1 && (mask & 1) && (mask >> (n - 1) & 1)) continue;
        std::int64_t s = 0;
        for (int i = 0; i < n; ++i)
            if (mask >> i & 1) s += m[i];
        best = std::max(best, s);
    }
    return best;
}

std::int64_t brute_delete_and_earn(const std::vector<int>& nums) {
    const std::set<int> distinct(nums.begin(), nums.end());
    const std::vector<int> vals(distinct.begin(), distinct.end());
    const int k = static_cast<int>(vals.size());
    std::int64_t best = 0;
    for (int mask = 0; mask < (1 << k); ++mask) {
        bool ok = true;
        std::int64_t s = 0;
        for (int i = 0; i < k && ok; ++i) {
            if (!(mask >> i & 1)) continue;
            for (int j = 0; j < k; ++j)
                if ((mask >> j & 1) && (vals[j] == vals[i] + 1)) ok = false;
            for (int x : nums)
                if (x == vals[i]) s += x;
        }
        if (ok) best = std::max(best, s);
    }
    return best;
}

}  // namespace

TEST(line_examples) {
    CHECK_EQ(dp::rob_line({}), 0);
    CHECK_EQ(dp::rob_line({5}), 5);
    CHECK_EQ(dp::rob_line({1, 2, 3, 1}), 4);
    CHECK_EQ(dp::rob_line({2, 7, 9, 3, 1}), 12);
    CHECK_EQ(dp::rob_line({2, 1, 1, 2}), 4);
}

TEST(line_matches_brute_force) {
    std::mt19937_64 rng(4);
    for (int iter = 0; iter < 500; ++iter) {
        V m(rng() % 16);
        for (auto& x : m) x = static_cast<std::int64_t>(rng() % 100);
        CHECK_EQ(dp::rob_line(m), brute(m, false));
    }
}

TEST(line_large) {
    V m(200000, 1'000'000'000);
    CHECK_EQ(dp::rob_line(m), 100000LL * 1'000'000'000LL);
}

TEST(circle_examples) {
    CHECK_EQ(dp::rob_circle({}), 0);
    CHECK_EQ(dp::rob_circle({7}), 7);
    CHECK_EQ(dp::rob_circle({3, 9}), 9);
    CHECK_EQ(dp::rob_circle({2, 3, 2}), 3);
    CHECK_EQ(dp::rob_circle({1, 2, 3, 1}), 4);
    CHECK_EQ(dp::rob_circle({1, 2, 3}), 3);
    CHECK_EQ(dp::rob_circle({200, 3, 140, 20, 10}), 340);
}

TEST(circle_matches_brute_force) {
    std::mt19937_64 rng(5);
    for (int iter = 0; iter < 500; ++iter) {
        V m(rng() % 16);
        for (auto& x : m) x = static_cast<std::int64_t>(rng() % 100);
        CHECK_EQ(dp::rob_circle(m), brute(m, true));
    }
}

TEST(delete_and_earn_examples) {
    CHECK_EQ(dp::delete_and_earn({3, 4, 2}), 6);
    CHECK_EQ(dp::delete_and_earn({2, 2, 3, 3, 3, 4}), 9);
    CHECK_EQ(dp::delete_and_earn({1}), 1);
    CHECK_EQ(dp::delete_and_earn({}), 0);
    CHECK_EQ(dp::delete_and_earn({100000, 1}), 100001);
}

TEST(delete_and_earn_matches_brute_force) {
    std::mt19937_64 rng(6);
    for (int iter = 0; iter < 300; ++iter) {
        std::vector<int> nums(rng() % 14);
        for (auto& x : nums) x = 1 + static_cast<int>(rng() % 12);
        CHECK_EQ(dp::delete_and_earn(nums), brute_delete_and_earn(nums));
    }
}

TEST(delete_and_earn_large) {
    std::vector<int> nums;
    for (int i = 0; i < 200000; ++i) nums.push_back(1 + i % 100000);  // each value twice
    // Best is all even values: 2 * (2 + 4 + ... + 100000) = 2 * 50000 * 50001
    CHECK_EQ(dp::delete_and_earn(nums), 2LL * 50000 * 50001);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
