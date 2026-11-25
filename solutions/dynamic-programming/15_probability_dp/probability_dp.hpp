// Problem 15: probability and expected-value DP. Reference solution.
#pragma once

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace dp {

inline double dice_sum_probability(int n, int faces, int target) {
    if (target < n || target > n * faces) return 0.0;
    std::vector<double> prob(target + 1, 0.0), next(target + 1);
    prob[0] = 1.0;  // zero dice: sum 0 with certainty
    for (int d = 1; d <= n; ++d) {
        std::fill(next.begin(), next.end(), 0.0);
        for (int s = 0; s <= target; ++s) {
            if (prob[s] == 0.0) continue;
            for (int f = 1; f <= faces && s + f <= target; ++f) next[s + f] += prob[s] / faces;
        }
        std::swap(prob, next);
    }
    return prob[target];
}

inline double optimal_dice_game_value(int rolls) {
    double v = 3.5;  // one roll left
    for (int k = 2; k <= rolls; ++k) {
        double total = 0;
        for (int x = 1; x <= 6; ++x) total += std::max(static_cast<double>(x), v);
        v = total / 6.0;
    }
    return v;
}

inline double expected_rolls_to_finish(int n) {
    std::vector<double> E(n + 1, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = 0;
        int valid = 0;
        for (int f = 1; f <= 6; ++f)
            if (i + f <= n) sum += E[i + f], ++valid;
        E[i] = (6.0 + sum) / valid;
    }
    return E[0];
}

inline double prob_run_of_heads(int n, int k, double p) {
    std::vector<double> cur(k, 0.0), next(k);
    cur[0] = 1.0;
    for (int i = 0; i < n; ++i) {
        std::fill(next.begin(), next.end(), 0.0);
        for (int r = 0; r < k; ++r) {
            next[0] += (1 - p) * cur[r];
            if (r + 1 < k) next[r + 1] += p * cur[r];  // r + 1 == k: run completed, mass leaves
        }
        std::swap(cur, next);
    }
    double no_run = 0;
    for (double x : cur) no_run += x;
    return 1.0 - no_run;
}

}  // namespace dp
