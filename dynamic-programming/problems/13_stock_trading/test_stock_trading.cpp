#include "stock_trading.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <random>
#include <tuple>
#include <vector>

#include "testing.hpp"

namespace {

using V = std::vector<std::int64_t>;

// Brute force over every action sequence (each day: do nothing, buy or sell), small n only.
// cooldown: no buy the day after a sell. Returns best profit with at most `k` buys.
std::int64_t brute(const V& p, std::size_t day, bool holding, bool sold_yesterday, std::int64_t k, std::int64_t fee,
                   bool cooldown) {
    if (day == p.size()) return holding ? INT64_MIN / 4 : 0;
    std::int64_t best = brute(p, day + 1, holding, false, k, fee, cooldown);
    if (holding)
        best = std::max(best, p[day] - fee + brute(p, day + 1, false, true, k, fee, cooldown));
    else if (k > 0 && !(cooldown && sold_yesterday))
        best = std::max(best, -p[day] + brute(p, day + 1, true, false, k - 1, fee, cooldown));
    return best;
}

V random_prices(std::mt19937_64& rng, std::size_t n) {
    V p(n);
    for (auto& x : p) x = static_cast<std::int64_t>(rng() % 20);
    return p;
}

}  // namespace

TEST(unlimited_examples) {
    CHECK_EQ(dp::max_profit_unlimited({7, 1, 5, 3, 6, 4}), 7);
    CHECK_EQ(dp::max_profit_unlimited({1, 2, 3, 4, 5}), 4);
    CHECK_EQ(dp::max_profit_unlimited({7, 6, 4, 3, 1}), 0);
    CHECK_EQ(dp::max_profit_unlimited({}), 0);
}

TEST(fee_examples) {
    CHECK_EQ(dp::max_profit_with_fee({1, 3, 2, 8, 4, 9}, 2), 8);
    CHECK_EQ(dp::max_profit_with_fee({1, 3, 7, 5, 10, 3}, 3), 6);
    CHECK_EQ(dp::max_profit_with_fee({1, 2}, 5), 0);
}

TEST(cooldown_examples) {
    CHECK_EQ(dp::max_profit_with_cooldown({1, 2, 3, 0, 2}), 3);
    CHECK_EQ(dp::max_profit_with_cooldown({1}), 0);
    CHECK_EQ(dp::max_profit_with_cooldown({}), 0);
    CHECK_EQ(dp::max_profit_with_cooldown({1, 2, 4}), 3);
    CHECK_EQ(dp::max_profit_with_cooldown({2, 1, 4, 5, 2, 9, 7}), 10);
}

TEST(k_trades_examples) {
    CHECK_EQ(dp::max_profit_k_trades({2, 4, 1}, 2), 2);
    CHECK_EQ(dp::max_profit_k_trades({3, 2, 6, 5, 0, 3}, 2), 7);
    CHECK_EQ(dp::max_profit_k_trades({3, 3, 5, 0, 0, 3, 1, 4}, 2), 6);
    CHECK_EQ(dp::max_profit_k_trades({1, 2, 4, 2, 5, 7, 2, 4, 9, 0}, 2), 13);
    CHECK_EQ(dp::max_profit_k_trades({1, 2, 4, 2, 5, 7, 2, 4, 9, 0}, 4), 15);
    CHECK_EQ(dp::max_profit_k_trades({1, 5}, 0), 0);
    CHECK_EQ(dp::max_profit_k_trades({1, 5}, 1'000'000'000), 4);
}

TEST(all_match_brute_force) {
    std::mt19937_64 rng(37);
    for (int iter = 0; iter < 400; ++iter) {
        V p = random_prices(rng, rng() % 11);
        std::int64_t fee = static_cast<std::int64_t>(rng() % 4);
        std::int64_t k = static_cast<std::int64_t>(rng() % 4);
        CHECK_EQ(dp::max_profit_unlimited(p), brute(p, 0, false, false, 100, 0, false));
        CHECK_EQ(dp::max_profit_with_fee(p, fee), brute(p, 0, false, false, 100, fee, false));
        CHECK_EQ(dp::max_profit_with_cooldown(p), brute(p, 0, false, false, 100, 0, true));
        CHECK_EQ(dp::max_profit_k_trades(p, k), brute(p, 0, false, false, k, 0, false));
    }
}

TEST(large_inputs) {
    V zigzag(200000);
    for (std::size_t i = 0; i < zigzag.size(); ++i) zigzag[i] = (i % 2 == 0) ? 0 : 1'000'000'000;
    CHECK_EQ(dp::max_profit_unlimited(zigzag), 100000LL * 1'000'000'000LL);
    CHECK_EQ(dp::max_profit_with_fee(zigzag, 999'999'999), 100000LL);
    // With cooldown the best rhythm is buy, sell, rest, wait: one round trip every 4 days.
    CHECK_EQ(dp::max_profit_with_cooldown(zigzag), 50000LL * 1'000'000'000LL);
    CHECK_EQ(dp::max_profit_k_trades(zigzag, 100), 100LL * 1'000'000'000LL);
    CHECK_EQ(dp::max_profit_k_trades(zigzag, 1'000'000'000), 100000LL * 1'000'000'000LL);
}

TEST(k_trades_medium_k) {
    std::mt19937_64 rng(38);
    V p(100000);
    for (auto& x : p) x = static_cast<std::int64_t>(rng() % 1000);
    std::int64_t prev = 0;
    for (std::int64_t k : {1, 2, 5, 50, 150}) {
        std::int64_t cur = dp::max_profit_k_trades(p, k);
        CHECK(cur >= prev);  // more trades never hurt
        prev = cur;
    }
    CHECK(prev <= dp::max_profit_unlimited(p));
    CHECK_EQ(dp::max_profit_k_trades(p, 1), 999);
    CHECK_EQ(dp::max_profit_k_trades(p, 150), 149646);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
