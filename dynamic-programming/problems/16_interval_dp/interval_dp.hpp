// Problem 16: interval DP with split points.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace dp {

// Matrix i (1-based) is dims[i-1] x dims[i]. Minimum scalar multiplications for the chain.
inline std::int64_t matrix_chain_cost(const std::vector<int>& dims) {
    (void)dims;
    throw std::logic_error("TODO: matrix_chain_cost");
}

// An optimal parenthesization, e.g. "((AB)C)". At most 26 matrices.
inline std::string matrix_chain_order(const std::vector<int>& dims) {
    (void)dims;
    throw std::logic_error("TODO: matrix_chain_order");
}

// Max coins from bursting all balloons (see README).
inline std::int64_t burst_balloons(const std::vector<int>& nums) {
    (void)nums;
    throw std::logic_error("TODO: burst_balloons");
}

// Min total cost to make all cuts in a stick of the given length; each cut costs the piece's length.
inline std::int64_t min_cut_cost(std::int64_t length, std::vector<std::int64_t> cuts) {
    (void)length;
    (void)cuts;
    throw std::logic_error("TODO: min_cut_cost");
}

}  // namespace dp
