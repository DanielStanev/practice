#include "probability_dp.hpp"

#include <cmath>
#include <random>
#include <vector>

#include "testing.hpp"

namespace {

constexpr double kEps = 1e-9;

// Enumerate every outcome of n dice.
double brute_dice(int n, int faces, int target) {
    std::vector<int> d(n, 1);
    long long hits = 0, total = 0;
    while (true) {
        int s = 0;
        for (int x : d) s += x;
        hits += s == target;
        ++total;
        int i = 0;
        while (i < n && d[i] == faces) d[i++] = 1;
        if (i == n) break;
        ++d[i];
    }
    return static_cast<double>(hits) / static_cast<double>(total);
}

// Value iteration: start E = 0 everywhere and apply the Bellman update until it converges.
double value_iteration_rolls(int n) {
    std::vector<double> E(n + 1, 0.0);
    for (int iter = 0; iter < 20000; ++iter)
        for (int i = n - 1; i >= 0; --i) {
            double s = 0;
            for (int f = 1; f <= 6; ++f) s += (i + f <= n) ? E[i + f] : E[i];
            E[i] = 1 + s / 6;
        }
    return E[0];
}

double brute_runs(int n, int k, double p) {
    double total = 0;
    for (int mask = 0; mask < (1 << n); ++mask) {
        int run = 0, best = 0, heads = 0;
        for (int i = 0; i < n; ++i) {
            if (mask >> i & 1) {
                ++heads;
                best = std::max(best, ++run);
            } else {
                run = 0;
            }
        }
        if (best >= k) total += std::pow(p, heads) * std::pow(1 - p, n - heads);
    }
    return total;
}

}  // namespace

TEST(dice_examples) {
    CHECK_NEAR(dp::dice_sum_probability(1, 6, 3), 1.0 / 6, kEps);
    CHECK_NEAR(dp::dice_sum_probability(2, 6, 7), 6.0 / 36, kEps);
    CHECK_NEAR(dp::dice_sum_probability(2, 6, 2), 1.0 / 36, kEps);
    CHECK_NEAR(dp::dice_sum_probability(2, 6, 13), 0.0, kEps);
    CHECK_NEAR(dp::dice_sum_probability(3, 6, 2), 0.0, kEps);
    CHECK_NEAR(dp::dice_sum_probability(0, 6, 0), 1.0, kEps);
    CHECK_NEAR(dp::dice_sum_probability(3, 6, 10), 27.0 / 216, kEps);
}

TEST(dice_matches_enumeration) {
    for (int n = 1; n <= 5; ++n)
        for (int faces : {2, 4, 6})
            for (int t = 0; t <= n * faces + 1; ++t)
                CHECK_NEAR(dp::dice_sum_probability(n, faces, t), brute_dice(n, faces, t), kEps);
}

TEST(dice_large_sums_to_one) {
    double total = 0, mean = 0;
    for (int t = 0; t <= 200 * 20; ++t) {
        double pr = dp::dice_sum_probability(200, 20, t);
        CHECK(pr >= 0);
        total += pr;
        mean += pr * t;
    }
    CHECK_NEAR(total, 1.0, 1e-9);
    CHECK_NEAR(mean, 200 * 10.5, 1e-6);
    CHECK_NEAR(dp::dice_sum_probability(100, 6, 600), std::pow(1.0 / 6, 100), 1e-90);
}

TEST(game_examples) {
    CHECK_NEAR(dp::optimal_dice_game_value(1), 3.5, kEps);
    CHECK_NEAR(dp::optimal_dice_game_value(2), 4.25, kEps);
    CHECK_NEAR(dp::optimal_dice_game_value(3), 14.0 / 3, kEps);
    CHECK_NEAR(dp::optimal_dice_game_value(4), 89.0 / 18, kEps);
}

TEST(game_monotone_and_bounded) {
    double prev = 0;
    for (int k = 1; k <= 1000; ++k) {
        double v = dp::optimal_dice_game_value(k);
        CHECK(v >= prev);
        CHECK(v < 6.0);
        prev = v;
    }
    CHECK(prev > 5.99);
}

TEST(rolls_examples) {
    CHECK_NEAR(dp::expected_rolls_to_finish(0), 0.0, kEps);
    CHECK_NEAR(dp::expected_rolls_to_finish(1), 6.0, kEps);  // need exactly a 1
    CHECK_NEAR(dp::expected_rolls_to_finish(2), 6.0, kEps);  // (6 + E[1] + E[2]) / 2
    CHECK_NEAR(dp::expected_rolls_to_finish(3), 6.0, kEps);
}

TEST(rolls_match_value_iteration) {
    for (int n = 0; n <= 20; ++n)
        CHECK_NEAR(dp::expected_rolls_to_finish(n), value_iteration_rolls(n), 1e-7);
}

TEST(rolls_large) {
    double e = dp::expected_rolls_to_finish(1000000);
    // Far from the end you move 3.5 per roll; the end game adds a constant.
    double e2 = dp::expected_rolls_to_finish(999993);
    CHECK_NEAR(e - e2, 7 / 3.5, 1e-6);
    CHECK(e > 1000000 / 3.5);
}

TEST(runs_examples) {
    CHECK_NEAR(dp::prob_run_of_heads(1, 1, 0.5), 0.5, kEps);
    CHECK_NEAR(dp::prob_run_of_heads(2, 2, 0.5), 0.25, kEps);
    CHECK_NEAR(dp::prob_run_of_heads(3, 2, 0.5), 3.0 / 8, kEps);
    CHECK_NEAR(dp::prob_run_of_heads(5, 6, 0.5), 0.0, kEps);
    CHECK_NEAR(dp::prob_run_of_heads(10, 1, 0.3), 1 - std::pow(0.7, 10), kEps);
    CHECK_NEAR(dp::prob_run_of_heads(0, 1, 0.5), 0.0, kEps);
}

TEST(runs_match_enumeration) {
    for (int n = 1; n <= 14; ++n)
        for (int k = 1; k <= n + 1; ++k)
            for (double p : {0.5, 0.3, 0.9})
                CHECK_NEAR(dp::prob_run_of_heads(n, k, p), brute_runs(n, k, p), kEps);
}

TEST(runs_large) {
    // Classic: 200 fair flips, P(run of at least 7 heads) is about 0.5 (the "longest run" rule of thumb log2(n)).
    double pr = dp::prob_run_of_heads(200, 7, 0.5);
    CHECK(pr > 0.5 && pr < 0.6);
    CHECK_NEAR(dp::prob_run_of_heads(20000, 200, 0.5), 0.0, 1e-50);
    CHECK_NEAR(dp::prob_run_of_heads(20000, 5, 0.5), 1.0, 1e-12);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
