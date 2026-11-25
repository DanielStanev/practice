// Problem 06: decode ways, word break, fewest words.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace dp {

inline constexpr std::int64_t kMod = 1'000'000'007;

// Number of ways to decode digits into letters (A=1 ... Z=26), modulo kMod.
inline std::int64_t num_decodings(std::string_view s) {
    (void)s;
    throw std::logic_error("TODO: num_decodings");
}

// Can s be split into a sequence of words from dict (reuse allowed)?
inline bool word_break(std::string_view s, const std::vector<std::string>& dict) {
    (void)s;
    (void)dict;
    throw std::logic_error("TODO: word_break");
}

// Fewest words in such a split, -1 if impossible, 0 for empty s.
inline int min_words(std::string_view s, const std::vector<std::string>& dict) {
    (void)s;
    (void)dict;
    throw std::logic_error("TODO: min_words");
}

}  // namespace dp
