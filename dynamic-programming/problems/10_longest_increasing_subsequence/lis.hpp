// Problem 10: longest increasing subsequence.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <stdexcept>
#include <utility>
#include <vector>

namespace dp {

// Length of the longest strictly increasing subsequence, O(n^2).
inline int lis_length_quadratic(const std::vector<int>& a) {
    (void)a;
    throw std::logic_error("TODO: lis_length_quadratic");
}

// Same, O(n log n).
inline int lis_length(const std::vector<int>& a) {
    (void)a;
    throw std::logic_error("TODO: lis_length");
}

// The values of one longest strictly increasing subsequence, O(n log n).
inline std::vector<int> lis_sequence(const std::vector<int>& a) {
    (void)a;
    throw std::logic_error("TODO: lis_sequence");
}

// Envelopes are {width, height}. Max chain where each fits strictly inside the next.
inline int max_envelopes(std::vector<std::pair<int, int>> envelopes) {
    (void)envelopes;
    throw std::logic_error("TODO: max_envelopes");
}

}  // namespace dp
