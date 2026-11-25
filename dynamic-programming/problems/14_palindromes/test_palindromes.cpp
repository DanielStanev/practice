#include "palindromes.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>

#include "testing.hpp"

namespace {

bool is_pal(const std::string& s) { return std::equal(s.begin(), s.end(), s.rbegin()); }

int brute_lps(const std::string& s) {
    int best = 0;
    for (int mask = 0; mask < (1 << s.size()); ++mask) {
        std::string t;
        for (std::size_t i = 0; i < s.size(); ++i)
            if (mask >> i & 1) t += s[i];
        if (is_pal(t)) best = std::max(best, static_cast<int>(t.size()));
    }
    return best;
}

int brute_cuts(const std::string& s) {
    // Try every set of cut positions.
    const int n = static_cast<int>(s.size());
    int best = n;
    for (int mask = 0; mask < (1 << (n - 1)); ++mask) {
        int start = 0, cuts = 0;
        bool ok = true;
        for (int i = 0; i < n && ok; ++i)
            if (i == n - 1 || (mask >> i & 1)) {
                ok = is_pal(s.substr(start, i - start + 1));
                start = i + 1;
                if (i != n - 1) ++cuts;
            }
        if (ok) best = std::min(best, cuts);
    }
    return best;
}

std::string random_string(std::mt19937_64& rng, std::size_t n, int alphabet) {
    std::string s(n, 'a');
    for (auto& c : s) c = static_cast<char>('a' + rng() % alphabet);
    return s;
}

}  // namespace

TEST(substring_examples) {
    auto r = dp::longest_palindromic_substring("babad");
    CHECK(r == "bab" || r == "aba");
    CHECK_EQ(dp::longest_palindromic_substring("cbbd"), std::string("bb"));
    CHECK_EQ(dp::longest_palindromic_substring("a"), std::string("a"));
    CHECK_EQ(dp::longest_palindromic_substring(""), std::string(""));
    CHECK_EQ(dp::longest_palindromic_substring("forgeeksskeegfor"), std::string("geeksskeeg"));
}

TEST(substring_is_valid_and_longest) {
    std::mt19937_64 rng(39);
    for (int iter = 0; iter < 500; ++iter) {
        auto s = random_string(rng, rng() % 25, 2);
        auto r = dp::longest_palindromic_substring(s);
        CHECK(is_pal(r));
        CHECK(s.find(r) != std::string::npos);
        std::size_t best = 0;
        for (std::size_t i = 0; i < s.size(); ++i)
            for (std::size_t len = 1; i + len <= s.size(); ++len)
                if (is_pal(s.substr(i, len))) best = std::max(best, len);
        CHECK_EQ(r.size(), best);
    }
}

TEST(count_examples) {
    CHECK_EQ(dp::count_palindromic_substrings("abc"), 3);
    CHECK_EQ(dp::count_palindromic_substrings("aaa"), 6);
    CHECK_EQ(dp::count_palindromic_substrings(""), 0);
    CHECK_EQ(dp::count_palindromic_substrings("abba"), 6);
}

TEST(count_matches_brute_force) {
    std::mt19937_64 rng(40);
    for (int iter = 0; iter < 500; ++iter) {
        auto s = random_string(rng, rng() % 25, 2);
        std::int64_t expect = 0;
        for (std::size_t i = 0; i < s.size(); ++i)
            for (std::size_t len = 1; i + len <= s.size(); ++len) expect += is_pal(s.substr(i, len));
        CHECK_EQ(dp::count_palindromic_substrings(s), expect);
    }
}

TEST(substring_and_count_large) {
    std::string s(5000, 'a');
    CHECK_EQ(dp::count_palindromic_substrings(s), 5000LL * 5001 / 2);
    CHECK_EQ(dp::longest_palindromic_substring(s).size(), 5000u);
    std::mt19937_64 rng(41);
    auto t = random_string(rng, 2000, 3);
    auto mid = t + std::string(t.rbegin(), t.rend());  // length 4000 palindrome
    auto u = random_string(rng, 500, 3) + "z" + mid + "y" + random_string(rng, 500, 3);
    CHECK_EQ(dp::longest_palindromic_substring(u), mid);
}

TEST(subsequence_examples) {
    CHECK_EQ(dp::longest_palindromic_subsequence("bbbab"), 4);
    CHECK_EQ(dp::longest_palindromic_subsequence("cbbd"), 2);
    CHECK_EQ(dp::longest_palindromic_subsequence("a"), 1);
    CHECK_EQ(dp::longest_palindromic_subsequence(""), 0);
    CHECK_EQ(dp::longest_palindromic_subsequence("character"), 5);  // carac
}

TEST(subsequence_matches_brute_force) {
    std::mt19937_64 rng(42);
    for (int iter = 0; iter < 400; ++iter) {
        auto s = random_string(rng, rng() % 14, 3);
        CHECK_EQ(dp::longest_palindromic_subsequence(s), brute_lps(s));
    }
}

TEST(subsequence_large) {
    std::string s;
    for (int i = 0; i < 3000; ++i) s += static_cast<char>('a' + i % 26);
    CHECK_EQ(dp::longest_palindromic_subsequence(s), 231);
    std::string p(3000, 'q');
    CHECK_EQ(dp::longest_palindromic_subsequence(p), 3000);
}

TEST(cuts_examples) {
    CHECK_EQ(dp::min_palindrome_cuts("aab"), 1);
    CHECK_EQ(dp::min_palindrome_cuts("a"), 0);
    CHECK_EQ(dp::min_palindrome_cuts("ab"), 1);
    CHECK_EQ(dp::min_palindrome_cuts("racecar"), 0);
    CHECK_EQ(dp::min_palindrome_cuts("abacdc"), 1);  // aba | cdc
    CHECK_EQ(dp::min_palindrome_cuts("abcde"), 4);
}

TEST(cuts_match_brute_force) {
    std::mt19937_64 rng(43);
    for (int iter = 0; iter < 400; ++iter) {
        auto s = random_string(rng, 1 + rng() % 13, 2);
        CHECK_EQ(dp::min_palindrome_cuts(s), brute_cuts(s));
    }
}

TEST(cuts_large) {
    std::string s(3000, 'a');
    CHECK_EQ(dp::min_palindrome_cuts(s), 0);
    std::string t;
    for (int i = 0; i < 3000; ++i) t += static_cast<char>('a' + i % 26);
    CHECK_EQ(dp::min_palindrome_cuts(t), 2999);
    std::string u;
    for (int i = 0; i < 750; ++i) u += "abbc";
    CHECK_EQ(dp::min_palindrome_cuts(u), 2249);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
