// Problem 01: Fibonacci four ways. Reference solution.
#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

namespace dp {

inline std::uint64_t fib_naive(int n) {
    return n < 2 ? static_cast<std::uint64_t>(n) : fib_naive(n - 1) + fib_naive(n - 2);
}

namespace detail {
inline std::uint64_t fib_memo_rec(int k, std::vector<std::optional<std::uint64_t>>& memo) {
    if (k < 2) return static_cast<std::uint64_t>(k);           // base cases
    if (memo[k]) return *memo[k];                               // already solved
    std::uint64_t v = fib_memo_rec(k - 1, memo) + fib_memo_rec(k - 2, memo);
    memo[k] = v;                                                // remember
    return v;
}
}  // namespace detail

inline std::uint64_t fib_memo(int n) {
    std::vector<std::optional<std::uint64_t>> memo(n + 1);
    return detail::fib_memo_rec(n, memo);
}

inline std::uint64_t fib_table(int n) {
    if (n < 2) return static_cast<std::uint64_t>(n);
    std::vector<std::uint64_t> t(n + 1);
    t[0] = 0;
    t[1] = 1;
    for (int k = 2; k <= n; ++k) t[k] = t[k - 1] + t[k - 2];
    return t[n];
}

inline std::uint64_t fib_two_vars(int n) {
    std::uint64_t prev = 0, cur = 1;  // F(k-1), F(k) with k = 1
    if (n == 0) return 0;
    for (int k = 2; k <= n; ++k) {
        std::uint64_t next = prev + cur;
        prev = cur;
        cur = next;
    }
    return cur;
}

// calls(k) = 1 for k < 2, else 1 + calls(k-1) + calls(k-2).
inline std::uint64_t naive_call_count(int n) {
    std::uint64_t a = 1, b = 1;  // calls(k-2), calls(k-1)
    if (n < 2) return 1;
    for (int k = 2; k <= n; ++k) {
        std::uint64_t c = 1 + a + b;
        a = b;
        b = c;
    }
    return b;
}

inline std::uint64_t tribonacci(int n) {
    std::uint64_t a = 0, b = 1, c = 1;  // T(k-2), T(k-1), T(k) for k = 2
    if (n == 0) return 0;
    if (n < 3) return 1;
    for (int k = 3; k <= n; ++k) {
        std::uint64_t d = a + b + c;
        a = b;
        b = c;
        c = d;
    }
    return c;
}

}  // namespace dp
