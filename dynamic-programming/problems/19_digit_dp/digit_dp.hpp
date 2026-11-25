// Problem 19: digit DP.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace dp {

// How many x in [lo, hi] have decimal digit sum exactly s. 0 <= lo <= hi <= 1e18.
inline std::int64_t count_digit_sum(std::int64_t lo, std::int64_t hi, int s) {
    (void)lo;
    (void)hi;
    (void)s;
    throw std::logic_error("TODO: count_digit_sum");
}

// How many x in [lo, hi] have no two equal adjacent decimal digits.
inline std::int64_t count_no_adjacent_equal(std::int64_t lo, std::int64_t hi) {
    (void)lo;
    (void)hi;
    throw std::logic_error("TODO: count_no_adjacent_equal");
}

// Total number of times digit d appears when writing every number in [lo, hi].
inline std::int64_t count_digit_occurrences(std::int64_t lo, std::int64_t hi, int d) {
    (void)lo;
    (void)hi;
    (void)d;
    throw std::logic_error("TODO: count_digit_occurrences");
}

}  // namespace dp
