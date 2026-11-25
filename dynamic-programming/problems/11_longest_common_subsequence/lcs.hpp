// Problem 11: longest common subsequence / substring.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace dp {

// Length of the longest common subsequence. |a|, |b| <= 5000.
inline int lcs_length(std::string_view a, std::string_view b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: lcs_length");
}

// One longest common subsequence. |a|, |b| <= 3000.
inline std::string lcs_string(std::string_view a, std::string_view b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: lcs_string");
}

// Length of the longest common contiguous substring. |a|, |b| <= 5000.
inline int longest_common_substring(std::string_view a, std::string_view b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: longest_common_substring");
}

}  // namespace dp
