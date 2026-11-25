// Problem 15: probability and expected-value DP.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <stdexcept>
#include <vector>

namespace dp {

// P(sum of n fair dice, each showing 1..faces uniformly, equals target).
inline double dice_sum_probability(int n, int faces, int target) {
    (void)n;
    (void)faces;
    (void)target;
    throw std::logic_error("TODO: dice_sum_probability");
}

// Value of: roll a d6, keep it or re-roll, at most `rolls` rolls in total, optimal play.
inline double optimal_dice_game_value(int rolls) {
    (void)rolls;
    throw std::logic_error("TODO: optimal_dice_game_value");
}

// Expected number of d6 rolls to move from square 0 to exactly square n,
// where a roll that would pass n is wasted (you stay put).
inline double expected_rolls_to_finish(int n) {
    (void)n;
    throw std::logic_error("TODO: expected_rolls_to_finish");
}

// P(at least one run of k consecutive heads in n flips), P(heads) = p.
inline double prob_run_of_heads(int n, int k, double p) {
    (void)n;
    (void)k;
    (void)p;
    throw std::logic_error("TODO: prob_run_of_heads");
}

}  // namespace dp
