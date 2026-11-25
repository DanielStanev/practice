// Problem 01: Fibonacci four ways.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dp {

// Given for reference: correct but exponential. Do not call it from your code.
inline std::uint64_t fib_naive(int n) {
    return n < 2 ? static_cast<std::uint64_t>(n) : fib_naive(n - 1) + fib_naive(n - 2);
}

// Top-down: recursion plus a cache. 0 <= n <= 93.
inline std::uint64_t fib_memo(int n) {
    (void)n;
    throw std::logic_error("TODO: fib_memo");
}

// Bottom-up: fill a std::vector from index 0 upward. 0 <= n <= 93.
inline std::uint64_t fib_table(int n) {
    (void)n;
    throw std::logic_error("TODO: fib_table");
}

// Bottom-up with O(1) extra memory. 0 <= n <= 93.
inline std::uint64_t fib_two_vars(int n) {
    (void)n;
    throw std::logic_error("TODO: fib_two_vars");
}

// Number of calls fib_naive(n) makes in total, including the first one.
// Do not call fib_naive. 0 <= n <= 70.
inline std::uint64_t naive_call_count(int n) {
    (void)n;
    throw std::logic_error("TODO: naive_call_count");
}

// T(0)=0, T(1)=1, T(2)=1, T(n)=T(n-1)+T(n-2)+T(n-3). 0 <= n <= 70.
inline std::uint64_t tribonacci(int n) {
    (void)n;
    throw std::logic_error("TODO: tribonacci");
}

}  // namespace dp
