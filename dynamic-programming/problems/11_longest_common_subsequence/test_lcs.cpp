#include "lcs.hpp"

#include <algorithm>
#include <random>
#include <string>

#include "testing.hpp"

namespace {

bool is_subsequence(const std::string& sub, const std::string& s) {
    std::size_t j = 0;
    for (std::size_t i = 0; i < s.size() && j < sub.size(); ++i)
        if (s[i] == sub[j]) ++j;
    return j == sub.size();
}

// Brute force: try every subsequence of the shorter string (length <= 12).
int brute_lcs(const std::string& a, const std::string& b) {
    const std::string& s = a.size() <= b.size() ? a : b;
    const std::string& t = a.size() <= b.size() ? b : a;
    int best = 0;
    for (int mask = 0; mask < (1 << s.size()); ++mask) {
        std::string sub;
        for (std::size_t i = 0; i < s.size(); ++i)
            if (mask >> i & 1) sub += s[i];
        if (static_cast<int>(sub.size()) > best && is_subsequence(sub, t)) best = static_cast<int>(sub.size());
    }
    return best;
}

int brute_substring(const std::string& a, const std::string& b) {
    int best = 0;
    for (std::size_t i = 0; i < a.size(); ++i)
        for (std::size_t len = 1; i + len <= a.size(); ++len)
            if (b.find(a.substr(i, len)) != std::string::npos) best = std::max(best, static_cast<int>(len));
    return best;
}

std::string random_string(std::mt19937_64& rng, std::size_t n, int alphabet) {
    std::string s(n, 'a');
    for (auto& c : s) c = static_cast<char>('a' + rng() % alphabet);
    return s;
}

}  // namespace

TEST(length_examples) {
    CHECK_EQ(dp::lcs_length("ABCBDAB", "BDCABA"), 4);
    CHECK_EQ(dp::lcs_length("abcde", "ace"), 3);
    CHECK_EQ(dp::lcs_length("abc", "def"), 0);
    CHECK_EQ(dp::lcs_length("", "abc"), 0);
    CHECK_EQ(dp::lcs_length("abc", ""), 0);
    CHECK_EQ(dp::lcs_length("same", "same"), 4);
}

TEST(length_matches_brute_force) {
    std::mt19937_64 rng(28);
    for (int iter = 0; iter < 500; ++iter) {
        auto a = random_string(rng, rng() % 12, 3);
        auto b = random_string(rng, rng() % 16, 3);
        CHECK_EQ(dp::lcs_length(a, b), brute_lcs(a, b));
    }
}

TEST(length_large) {
    std::string a(5000, 'a'), b(5000, 'b');
    CHECK_EQ(dp::lcs_length(a, b), 0);
    for (int i = 0; i < 5000; i += 2) b[i] = 'a';
    CHECK_EQ(dp::lcs_length(a, b), 2500);
    std::mt19937_64 rng(29);
    auto x = random_string(rng, 5000, 4);
    auto y = random_string(rng, 4000, 4);
    CHECK_EQ(dp::lcs_length(x, y), dp::lcs_length(y, x));
}

TEST(string_examples) {
    CHECK_EQ(dp::lcs_string("abcde", "ace"), std::string("ace"));
    CHECK_EQ(dp::lcs_string("abc", "def"), std::string(""));
    CHECK_EQ(dp::lcs_string("", ""), std::string(""));
    CHECK_EQ(dp::lcs_string("xyz", "xyz"), std::string("xyz"));
    CHECK_EQ(dp::lcs_string("ABCBDAB", "BDCABA").size(), 4u);
}

TEST(string_is_valid) {
    std::mt19937_64 rng(30);
    for (int iter = 0; iter < 500; ++iter) {
        auto a = random_string(rng, rng() % 12, 3);
        auto b = random_string(rng, rng() % 16, 3);
        auto s = dp::lcs_string(a, b);
        CHECK_EQ(static_cast<int>(s.size()), brute_lcs(a, b));
        CHECK(is_subsequence(s, a));
        CHECK(is_subsequence(s, b));
    }
}

TEST(string_large) {
    std::mt19937_64 rng(31);
    auto a = random_string(rng, 3000, 4);
    auto b = random_string(rng, 3000, 4);
    auto s = dp::lcs_string(a, b);
    CHECK_EQ(static_cast<int>(s.size()), dp::lcs_length(a, b));
    CHECK(is_subsequence(s, a));
    CHECK(is_subsequence(s, b));
}

TEST(substring_examples) {
    CHECK_EQ(dp::longest_common_substring("abcdxyz", "xyzabcd"), 4);
    CHECK_EQ(dp::longest_common_substring("zxabcdezy", "yzabcdezx"), 6);
    CHECK_EQ(dp::longest_common_substring("abc", "def"), 0);
    CHECK_EQ(dp::longest_common_substring("", "a"), 0);
    CHECK_EQ(dp::longest_common_substring("ace", "abcde"), 1);  // subsequence would be 3
}

TEST(substring_matches_brute_force) {
    std::mt19937_64 rng(32);
    for (int iter = 0; iter < 500; ++iter) {
        auto a = random_string(rng, rng() % 20, 2);
        auto b = random_string(rng, rng() % 20, 2);
        CHECK_EQ(dp::longest_common_substring(a, b), brute_substring(a, b));
    }
}

TEST(substring_large) {
    std::string a(5000, 'a'), b(5000, 'a');
    CHECK_EQ(dp::longest_common_substring(a, b), 5000);
    b[2000] = 'b';
    CHECK_EQ(dp::longest_common_substring(a, b), 2999);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
