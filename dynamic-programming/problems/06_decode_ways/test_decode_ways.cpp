#include "decode_ways.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "testing.hpp"

namespace {

// Brute force by recursion over the remaining suffix.
std::int64_t brute_decode(const std::string& s, std::size_t pos) {
    if (pos == s.size()) return 1;
    std::int64_t total = 0;
    if (s[pos] != '0') total += brute_decode(s, pos + 1);
    if (pos + 1 < s.size() && s[pos] != '0' && (s[pos] - '0') * 10 + (s[pos + 1] - '0') <= 26)
        total += brute_decode(s, pos + 2);
    return total;
}

int brute_words(const std::string& s, std::size_t pos, const std::vector<std::string>& dict) {
    if (pos == s.size()) return 0;
    int best = -1;
    for (const auto& w : dict)
        if (s.compare(pos, w.size(), w) == 0) {
            int r = brute_words(s, pos + w.size(), dict);
            if (r >= 0 && (best < 0 || r + 1 < best)) best = r + 1;
        }
    return best;
}

}  // namespace

TEST(decode_examples) {
    CHECK_EQ(dp::num_decodings("12"), 2);
    CHECK_EQ(dp::num_decodings("226"), 3);
    CHECK_EQ(dp::num_decodings("2101"), 1);
    CHECK_EQ(dp::num_decodings(""), 1);
    CHECK_EQ(dp::num_decodings("27"), 1);
    CHECK_EQ(dp::num_decodings("11106"), 2);
}

TEST(decode_zeros) {
    CHECK_EQ(dp::num_decodings("0"), 0);
    CHECK_EQ(dp::num_decodings("06"), 0);
    CHECK_EQ(dp::num_decodings("00"), 0);
    CHECK_EQ(dp::num_decodings("30"), 0);
    CHECK_EQ(dp::num_decodings("100"), 0);
    CHECK_EQ(dp::num_decodings("10"), 1);
    CHECK_EQ(dp::num_decodings("20"), 1);
    CHECK_EQ(dp::num_decodings("1201234"), 3);
}

TEST(decode_matches_brute_force) {
    std::mt19937_64 rng(11);
    const std::string alphabet = "0112223456789";  // more 1s and 2s -> more branching
    for (int iter = 0; iter < 2000; ++iter) {
        std::string s(rng() % 20, '1');
        for (auto& c : s) c = alphabet[rng() % alphabet.size()];
        CHECK_EQ(dp::num_decodings(s), brute_decode(s, 0));
    }
}

TEST(decode_large_uses_modulo) {
    CHECK_EQ(dp::num_decodings(std::string(200000, '1')), 835799900);
    std::string s;
    for (int i = 0; i < 50000; ++i) s += "1210";
    CHECK_EQ(dp::num_decodings(s), 18165451);
}

TEST(word_break_examples) {
    CHECK(dp::word_break("leetcode", {"leet", "code"}));
    CHECK(dp::word_break("applepenapple", {"apple", "pen"}));
    CHECK_FALSE(dp::word_break("catsandog", {"cats", "dog", "sand", "and", "cat"}));
    CHECK(dp::word_break("", {"a"}));
    CHECK_FALSE(dp::word_break("a", {}));
}

TEST(min_words_examples) {
    CHECK_EQ(dp::min_words("leetcode", {"leet", "code", "l", "e", "t", "c", "o", "d"}), 2);
    CHECK_EQ(dp::min_words("aaaaaaa", {"a", "aaa"}), 3);
    CHECK_EQ(dp::min_words("catsandog", {"cats", "dog", "sand", "and", "cat"}), -1);
    CHECK_EQ(dp::min_words("", {"x"}), 0);
}

TEST(words_match_brute_force) {
    std::mt19937_64 rng(12);
    for (int iter = 0; iter < 600; ++iter) {
        std::vector<std::string> dict(1 + rng() % 5);
        for (auto& w : dict) {
            w.assign(1 + rng() % 3, 'a');
            for (auto& c : w) c = static_cast<char>('a' + rng() % 2);
        }
        std::string s(rng() % 14, 'a');
        for (auto& c : s) c = static_cast<char>('a' + rng() % 2);
        int expect = brute_words(s, 0, dict);
        CHECK_EQ(dp::min_words(s, dict), expect);
        CHECK_EQ(dp::word_break(s, dict), expect >= 0);
    }
}

TEST(words_large) {
    // 100000 'a's then 'b': classic worst case for naive recursion.
    std::string s(100000, 'a');
    std::vector<std::string> dict;
    for (int len = 1; len <= 20; ++len) dict.push_back(std::string(len, 'a'));
    CHECK_EQ(dp::min_words(s, dict), 5000);
    s += 'b';
    CHECK_FALSE(dp::word_break(s, dict));
    CHECK_EQ(dp::min_words(s, dict), -1);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
