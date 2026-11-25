#include "knapsack.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <set>
#include <vector>

#include "testing.hpp"

using dp::Item;

namespace {

std::int64_t brute(const std::vector<Item>& items, std::int64_t cap) {
    const int n = static_cast<int>(items.size());
    std::int64_t best = 0;
    for (int mask = 0; mask < (1 << n); ++mask) {
        std::int64_t w = 0, v = 0;
        for (int i = 0; i < n; ++i)
            if (mask >> i & 1) w += items[i].weight, v += items[i].value;
        if (w <= cap) best = std::max(best, v);
    }
    return best;
}

bool brute_partition(const std::vector<int>& nums) {
    const int n = static_cast<int>(nums.size());
    for (int mask = 0; mask < (1 << n); ++mask) {
        int a = 0, b = 0;
        for (int i = 0; i < n; ++i) (mask >> i & 1 ? a : b) += nums[i];
        if (a == b) return true;
    }
    return false;
}

std::vector<Item> random_items(std::mt19937_64& rng, int n, int max_w, int max_v) {
    std::vector<Item> items(n);
    for (auto& it : items) {
        it.weight = 1 + static_cast<std::int64_t>(rng() % max_w);
        it.value = static_cast<std::int64_t>(rng() % (max_v + 1));
    }
    return items;
}

}  // namespace

TEST(knapsack_examples) {
    CHECK_EQ(dp::knapsack({{3, 30}, {4, 50}, {5, 60}}, 8), 90);
    CHECK_EQ(dp::knapsack({{1, 1}, {3, 4}, {4, 5}, {5, 7}}, 7), 9);
    CHECK_EQ(dp::knapsack({}, 10), 0);
    CHECK_EQ(dp::knapsack({{5, 10}}, 4), 0);
    CHECK_EQ(dp::knapsack({{2, 3}}, 100), 3);  // each item only once
}

TEST(knapsack_matches_brute_force) {
    std::mt19937_64 rng(17);
    for (int iter = 0; iter < 300; ++iter) {
        auto items = random_items(rng, static_cast<int>(rng() % 12), 15, 50);
        int cap = static_cast<int>(rng() % 60);
        CHECK_EQ(dp::knapsack(items, cap), brute(items, cap));
    }
}

TEST(knapsack_large) {
    std::mt19937_64 rng(18);
    auto items = random_items(rng, 200, 2000, 1'000'000'000);
    CHECK_EQ(dp::knapsack(items, 100000), 80122707008LL);
}

TEST(items_are_valid_and_optimal) {
    std::mt19937_64 rng(19);
    for (int iter = 0; iter < 300; ++iter) {
        auto items = random_items(rng, static_cast<int>(rng() % 12), 15, 50);
        int cap = static_cast<int>(rng() % 60);
        auto chosen = dp::knapsack_items(items, cap);
        CHECK(std::is_sorted(chosen.begin(), chosen.end()));
        CHECK_EQ(std::set<int>(chosen.begin(), chosen.end()).size(), chosen.size());
        std::int64_t w = 0, v = 0;
        for (int i : chosen) {
            CHECK(i >= 0 && i < static_cast<int>(items.size()));
            w += items[i].weight;
            v += items[i].value;
        }
        CHECK(w <= cap);
        CHECK_EQ(v, brute(items, cap));
    }
}

TEST(partition_examples) {
    CHECK(dp::can_partition_equal({1, 5, 11, 5}));
    CHECK_FALSE(dp::can_partition_equal({1, 2, 3, 5}));
    CHECK(dp::can_partition_equal({}));
    CHECK_FALSE(dp::can_partition_equal({7}));
    CHECK(dp::can_partition_equal({2, 2}));
    CHECK_FALSE(dp::can_partition_equal({1, 1, 1}));
}

TEST(partition_matches_brute_force) {
    std::mt19937_64 rng(20);
    for (int iter = 0; iter < 500; ++iter) {
        std::vector<int> nums(rng() % 14);
        for (auto& x : nums) x = 1 + static_cast<int>(rng() % 20);
        CHECK_EQ(dp::can_partition_equal(nums), brute_partition(nums));
    }
}

TEST(partition_large) {
    std::vector<int> nums(200, 1000);
    CHECK(dp::can_partition_equal(nums));
    nums[0] = 999;
    nums[1] = 997;  // even total 199996, but no subset reaches 99998
    CHECK_FALSE(dp::can_partition_equal(nums));
    nums[1] = 999;  // put one 999 on each side
    CHECK(dp::can_partition_equal(nums));
    std::vector<int> mixed;
    for (int i = 1; i <= 200; ++i) mixed.push_back(i * 5 % 1000 + 1);
    CHECK_EQ(dp::can_partition_equal(mixed), true);
}

TEST(huge_capacity_examples) {
    CHECK_EQ(dp::knapsack_huge_capacity({{3, 30}, {4, 50}, {5, 60}}, 8), 90);
    CHECK_EQ(dp::knapsack_huge_capacity({{1'000'000'000, 10}}, 1'000'000'000), 10);
    CHECK_EQ(dp::knapsack_huge_capacity({{1'000'000'000, 10}}, 999'999'999), 0);
    CHECK_EQ(dp::knapsack_huge_capacity({{6, 5}, {5, 6}, {6, 4}, {6, 6}, {3, 5}, {7, 2}}, 15), 17);
}

TEST(huge_capacity_matches_brute_force) {
    std::mt19937_64 rng(21);
    for (int iter = 0; iter < 300; ++iter) {
        std::vector<Item> items(rng() % 12);
        for (auto& it : items) {
            it.weight = 1 + static_cast<std::int64_t>(rng() % 1'000'000'000);
            it.value = static_cast<std::int64_t>(rng() % 1001);
        }
        std::int64_t cap = static_cast<std::int64_t>(rng() % 3'000'000'000ULL);
        CHECK_EQ(dp::knapsack_huge_capacity(items, cap), brute(items, cap));
    }
}

TEST(huge_capacity_large) {
    std::mt19937_64 rng(22);
    std::vector<Item> items(100);
    for (auto& it : items) {
        it.weight = 1 + static_cast<std::int64_t>(rng() % 1'000'000'000);
        it.value = static_cast<std::int64_t>(rng() % 1001);
    }
    CHECK_EQ(dp::knapsack_huge_capacity(items, 1'000'000'000), 10308);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
