#include "coin_change.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include "testing.hpp"

namespace {

// Brute force: choose a count for each coin in order (combinations).
void brute_comb(const std::vector<int>& coins, std::size_t i, int left, int used, std::int64_t& count, int& fewest) {
    if (i == coins.size()) {
        if (left == 0) {
            ++count;
            if (fewest < 0 || used < fewest) fewest = used;
        }
        return;
    }
    for (int k = 0; k * coins[i] <= left; ++k) brute_comb(coins, i + 1, left - k * coins[i], used + k, count, fewest);
}

std::int64_t brute_perm(const std::vector<int>& coins, int left) {
    if (left == 0) return 1;
    std::int64_t total = 0;
    for (int c : coins)
        if (c <= left) total += brute_perm(coins, left - c);
    return total;
}

}  // namespace

TEST(min_coins_examples) {
    CHECK_EQ(dp::min_coins({1, 2, 5}, 11), 3);
    CHECK_EQ(dp::min_coins({2}, 3), -1);
    CHECK_EQ(dp::min_coins({1}, 0), 0);
    CHECK_EQ(dp::min_coins({1, 3, 4}, 6), 2);  // greedy would say 3
    CHECK_EQ(dp::min_coins({186, 419, 83, 408}, 6249), 20);
    CHECK_EQ(dp::min_coins({7}, 100000), -1);
}

TEST(combinations_examples) {
    CHECK_EQ(dp::count_combinations({1, 2}, 3), 2);
    CHECK_EQ(dp::count_combinations({1, 2, 5}, 5), 4);
    CHECK_EQ(dp::count_combinations({2}, 3), 0);
    CHECK_EQ(dp::count_combinations({10}, 10), 1);
    CHECK_EQ(dp::count_combinations({3, 5}, 0), 1);
}

TEST(permutations_examples) {
    CHECK_EQ(dp::count_permutations({1, 2}, 3), 3);
    CHECK_EQ(dp::count_permutations({1, 2, 3}, 4), 7);
    CHECK_EQ(dp::count_permutations({9}, 3), 0);
    CHECK_EQ(dp::count_permutations({3, 5}, 0), 1);
}

TEST(all_match_brute_force) {
    std::mt19937_64 rng(16);
    for (int iter = 0; iter < 300; ++iter) {
        std::vector<int> coins;
        int k = 1 + static_cast<int>(rng() % 4);
        while (static_cast<int>(coins.size()) < k) {
            int c = 1 + static_cast<int>(rng() % 9);
            if (std::find(coins.begin(), coins.end(), c) == coins.end()) coins.push_back(c);
        }
        int amount = static_cast<int>(rng() % 25);
        std::int64_t count = 0;
        int fewest = -1;
        brute_comb(coins, 0, amount, 0, count, fewest);
        CHECK_EQ(dp::min_coins(coins, amount), fewest);
        CHECK_EQ(dp::count_combinations(coins, amount), count);
        CHECK_EQ(dp::count_permutations(coins, amount), brute_perm(coins, amount));
    }
}

TEST(large_inputs) {
    std::vector<int> coins;
    for (int c = 1; c <= 100; ++c) coins.push_back(c * 7 + 3);  // 10, 17, ..., 703
    CHECK_EQ(dp::min_coins(coins, 100000), 144);
    CHECK_EQ(dp::count_combinations(coins, 100000), 155653386);
    CHECK_EQ(dp::count_permutations(coins, 100000), 517107340);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
