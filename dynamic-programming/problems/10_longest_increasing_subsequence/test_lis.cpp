#include "lis.hpp"

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

#include "testing.hpp"

namespace {

int brute_lis(const std::vector<int>& a) {
    const int n = static_cast<int>(a.size());
    int best = 0;
    for (int mask = 0; mask < (1 << n); ++mask) {
        int last = 0, len = 0;
        bool ok = true;
        for (int i = 0; i < n && ok; ++i) {
            if (!(mask >> i & 1)) continue;
            if (len > 0 && a[i] <= last) ok = false;
            last = a[i];
            ++len;
        }
        if (ok) best = std::max(best, len);
    }
    return best;
}

bool is_subsequence(const std::vector<int>& sub, const std::vector<int>& a) {
    std::size_t j = 0;
    for (std::size_t i = 0; i < a.size() && j < sub.size(); ++i)
        if (a[i] == sub[j]) ++j;
    return j == sub.size();
}

int brute_envelopes(const std::vector<std::pair<int, int>>& e) {
    // O(n^2) DP over envelopes sorted by width, independent of the LIS trick.
    auto s = e;
    std::sort(s.begin(), s.end());
    std::vector<int> len(s.size(), 1);
    int best = 0;
    for (std::size_t i = 0; i < s.size(); ++i) {
        for (std::size_t j = 0; j < i; ++j)
            if (s[j].first < s[i].first && s[j].second < s[i].second) len[i] = std::max(len[i], len[j] + 1);
        best = std::max(best, len[i]);
    }
    return best;
}

std::vector<int> random_vec(std::mt19937_64& rng, int n, int range) {
    std::vector<int> a(n);
    for (auto& x : a) x = static_cast<int>(rng() % range) - range / 2;
    return a;
}

}  // namespace

TEST(quadratic_examples) {
    CHECK_EQ(dp::lis_length_quadratic({10, 9, 2, 5, 3, 7, 101, 18}), 4);
    CHECK_EQ(dp::lis_length_quadratic({0, 1, 0, 3, 2, 3}), 4);
    CHECK_EQ(dp::lis_length_quadratic({7, 7, 7, 7}), 1);
    CHECK_EQ(dp::lis_length_quadratic({}), 0);
}

TEST(fast_examples) {
    CHECK_EQ(dp::lis_length({10, 9, 2, 5, 3, 7, 101, 18}), 4);
    CHECK_EQ(dp::lis_length({0, 1, 0, 3, 2, 3}), 4);
    CHECK_EQ(dp::lis_length({7, 7, 7, 7}), 1);
    CHECK_EQ(dp::lis_length({}), 0);
    CHECK_EQ(dp::lis_length({-5, -3, -4, -1}), 3);
}

TEST(lengths_match_brute_force) {
    std::mt19937_64 rng(23);
    for (int iter = 0; iter < 1000; ++iter) {
        auto a = random_vec(rng, static_cast<int>(rng() % 15), 12);
        int expect = brute_lis(a);
        CHECK_EQ(dp::lis_length_quadratic(a), expect);
        CHECK_EQ(dp::lis_length(a), expect);
    }
}

TEST(quadratic_matches_fast_medium) {
    std::mt19937_64 rng(24);
    for (int iter = 0; iter < 5; ++iter) {
        auto a = random_vec(rng, 2000, 1000);
        CHECK_EQ(dp::lis_length_quadratic(a), dp::lis_length(a));
    }
}

TEST(fast_large) {
    std::vector<int> inc(300000);
    for (int i = 0; i < 300000; ++i) inc[i] = i - 150000;
    CHECK_EQ(dp::lis_length(inc), 300000);
    std::reverse(inc.begin(), inc.end());
    CHECK_EQ(dp::lis_length(inc), 1);
    std::vector<int> saw;
    for (int i = 0; i < 300000; ++i) saw.push_back(i % 1000);  // 0..999 repeated
    CHECK_EQ(dp::lis_length(saw), 1000);
}

TEST(sequence_examples) {
    auto s = dp::lis_sequence({10, 9, 2, 5, 3, 7, 101, 18});
    CHECK_EQ(s.size(), 4u);
    CHECK(dp::lis_sequence({}).empty());
    CHECK(dp::lis_sequence({5}) == std::vector<int>{5});
    CHECK(dp::lis_sequence({3, 1, 2}) == (std::vector<int>{1, 2}));
}

TEST(sequence_is_valid) {
    std::mt19937_64 rng(25);
    for (int iter = 0; iter < 1000; ++iter) {
        auto a = random_vec(rng, static_cast<int>(rng() % 15), 12);
        auto s = dp::lis_sequence(a);
        CHECK_EQ(static_cast<int>(s.size()), brute_lis(a));
        CHECK(std::adjacent_find(s.begin(), s.end(), [](int x, int y) { return x >= y; }) == s.end());
        CHECK(is_subsequence(s, a));
    }
}

TEST(sequence_large) {
    std::mt19937_64 rng(26);
    auto a = random_vec(rng, 300000, 2'000'000'000);
    auto s = dp::lis_sequence(a);
    CHECK_EQ(static_cast<int>(s.size()), dp::lis_length(a));
    CHECK(std::adjacent_find(s.begin(), s.end(), [](int x, int y) { return x >= y; }) == s.end());
    CHECK(is_subsequence(s, a));
}

TEST(envelopes_examples) {
    CHECK_EQ(dp::max_envelopes({{5, 4}, {6, 4}, {6, 7}, {2, 3}}), 3);
    CHECK_EQ(dp::max_envelopes({{1, 1}, {1, 1}, {1, 1}}), 1);
    CHECK_EQ(dp::max_envelopes({{1, 2}, {1, 3}, {1, 4}}), 1);  // same width never nests
    CHECK_EQ(dp::max_envelopes({}), 0);
}

TEST(envelopes_match_brute_force) {
    std::mt19937_64 rng(27);
    for (int iter = 0; iter < 500; ++iter) {
        std::vector<std::pair<int, int>> e(rng() % 30);
        for (auto& [w, h] : e) w = 1 + static_cast<int>(rng() % 8), h = 1 + static_cast<int>(rng() % 8);
        CHECK_EQ(dp::max_envelopes(e), brute_envelopes(e));
    }
}

TEST(envelopes_large) {
    std::vector<std::pair<int, int>> e;
    for (int i = 0; i < 200000; ++i) e.push_back({i / 2, i / 2});  // pairs of identical envelopes
    CHECK_EQ(dp::max_envelopes(e), 100000);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
