// Problem 14: palindromes (interval DP).
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace dp {

// One longest palindromic substring. Empty input -> "".
inline std::string longest_palindromic_substring(std::string_view s) {
    (void)s;
    throw std::logic_error("TODO: longest_palindromic_substring");
}

// Number of index pairs (i <= j) such that s[i..j] is a palindrome.
inline std::int64_t count_palindromic_substrings(std::string_view s) {
    (void)s;
    throw std::logic_error("TODO: count_palindromic_substrings");
}

// Length of the longest palindromic subsequence.
inline int longest_palindromic_subsequence(std::string_view s) {
    (void)s;
    throw std::logic_error("TODO: longest_palindromic_subsequence");
}

// Fewest cuts so that every piece is a palindrome. s is non-empty.
inline int min_palindrome_cuts(std::string_view s) {
    (void)s;
    throw std::logic_error("TODO: min_palindrome_cuts");
}

}  // namespace dp
