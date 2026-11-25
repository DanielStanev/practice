#include "digit_dp.hpp"

#include <cstdint>
#include <random>
#include <string>

#include "testing.hpp"

namespace {

int digit_sum(std::int64_t x) {
    int s = 0;
    for (char c : std::to_string(x)) s += c - '0';
    return s;
}

bool no_adjacent(std::int64_t x) {
    auto s = std::to_string(x);
    for (std::size_t i = 1; i < s.size(); ++i)
        if (s[i] == s[i - 1]) return false;
    return true;
}

std::int64_t occurrences(std::int64_t x, int d) {
    std::int64_t c = 0;
    for (char ch : std::to_string(x)) c += ch - '0' == d;
    return c;
}

constexpr std::int64_t kE18 = 1'000'000'000'000'000'000LL;

}  // namespace

TEST(digit_sum_examples) {
    CHECK_EQ(dp::count_digit_sum(0, 9, 5), 1);
    CHECK_EQ(dp::count_digit_sum(0, 99, 5), 6);  // 5 14 23 32 41 50
    CHECK_EQ(dp::count_digit_sum(0, 0, 0), 1);
    CHECK_EQ(dp::count_digit_sum(1, 100, 1), 3);  // 1 10 100
    CHECK_EQ(dp::count_digit_sum(0, 999, 27), 1);
    CHECK_EQ(dp::count_digit_sum(0, 999, 28), 0);
    CHECK_EQ(dp::count_digit_sum(5, 5, 5), 1);
}

TEST(digit_sum_matches_brute_force) {
    std::mt19937_64 rng(56);
    for (int iter = 0; iter < 300; ++iter) {
        std::int64_t lo = static_cast<std::int64_t>(rng() % 5000), hi = lo + static_cast<std::int64_t>(rng() % 5000);
        int s = static_cast<int>(rng() % 30);
        std::int64_t expect = 0;
        for (std::int64_t x = lo; x <= hi; ++x) expect += digit_sum(x) == s;
        CHECK_EQ(dp::count_digit_sum(lo, hi, s), expect);
    }
}

TEST(digit_sum_large) {
    CHECK_EQ(dp::count_digit_sum(0, kE18, 1), 19);
    CHECK_EQ(dp::count_digit_sum(0, kE18 - 1, 162), 1);  // 999...9 (18 nines)
    CHECK_EQ(dp::count_digit_sum(0, kE18, 200), 0);
    std::int64_t total = 0;
    for (int s = 0; s <= 162; ++s) total += dp::count_digit_sum(123456789, kE18, s);
    CHECK_EQ(total, kE18 - 123456789 + 1);
    CHECK_EQ(dp::count_digit_sum(0, kE18 - 1, 81), 32458256583753952LL);
}

TEST(no_adjacent_examples) {
    CHECK_EQ(dp::count_no_adjacent_equal(0, 9), 10);
    CHECK_EQ(dp::count_no_adjacent_equal(10, 99), 81);
    CHECK_EQ(dp::count_no_adjacent_equal(11, 11), 0);
    CHECK_EQ(dp::count_no_adjacent_equal(100, 100), 0);
    CHECK_EQ(dp::count_no_adjacent_equal(101, 101), 1);
    CHECK_EQ(dp::count_no_adjacent_equal(0, 0), 1);
}

TEST(no_adjacent_matches_brute_force) {
    std::mt19937_64 rng(57);
    for (int iter = 0; iter < 300; ++iter) {
        std::int64_t lo = static_cast<std::int64_t>(rng() % 20000), hi = lo + static_cast<std::int64_t>(rng() % 5000);
        std::int64_t expect = 0;
        for (std::int64_t x = lo; x <= hi; ++x) expect += no_adjacent(x);
        CHECK_EQ(dp::count_no_adjacent_equal(lo, hi), expect);
    }
}

TEST(no_adjacent_large) {
    // k-digit numbers with no equal neighbours: 9 * 9^(k-1) = 9^k.
    std::int64_t expect = 1;  // the number 0
    std::int64_t p = 1;
    for (int k = 1; k <= 18; ++k) expect += (p *= 9);
    CHECK_EQ(dp::count_no_adjacent_equal(0, kE18 - 1), expect);
    CHECK_EQ(dp::count_no_adjacent_equal(0, kE18), expect);  // 10^18 has adjacent zeros
}

TEST(occurrences_examples) {
    CHECK_EQ(dp::count_digit_occurrences(0, 13, 1), 6);  // 1 10 11(2) 12 13
    CHECK_EQ(dp::count_digit_occurrences(0, 0, 0), 1);
    CHECK_EQ(dp::count_digit_occurrences(1, 9, 0), 0);
    CHECK_EQ(dp::count_digit_occurrences(1, 100, 0), 11);  // 10 20 ... 90 (9) + 100 (2)
    CHECK_EQ(dp::count_digit_occurrences(0, 99, 7), 20);
}

TEST(occurrences_match_brute_force) {
    std::mt19937_64 rng(58);
    for (int iter = 0; iter < 300; ++iter) {
        std::int64_t lo = static_cast<std::int64_t>(rng() % 20000), hi = lo + static_cast<std::int64_t>(rng() % 3000);
        int d = static_cast<int>(rng() % 10);
        std::int64_t expect = 0;
        for (std::int64_t x = lo; x <= hi; ++x) expect += occurrences(x, d);
        CHECK_EQ(dp::count_digit_occurrences(lo, hi, d), expect);
    }
}

TEST(occurrences_large) {
    // In [0, 10^18 - 1] padded to 18 digits, every digit appears 18 * 10^17 times.
    CHECK_EQ(dp::count_digit_occurrences(0, kE18 - 1, 7), 18 * (kE18 / 10));
    CHECK_EQ(dp::count_digit_occurrences(0, kE18, 1), 18 * (kE18 / 10) + 1);
    std::int64_t total = 0;
    for (int d = 0; d <= 9; ++d) total += dp::count_digit_occurrences(1, 999'999, d);
    // 9 one-digit, 90 two-digit, ... 900000 six-digit numbers.
    CHECK_EQ(total, 9 * 1 + 90 * 2 + 900 * 3 + 9000 * 4 + 90000 * 5 + 900000 * 6);
}

TEST(many_calls_are_fast) {
    std::mt19937_64 rng(59);
    std::uint64_t sink = 0;
    for (int iter = 0; iter < 2000; ++iter) {
        std::int64_t lo = static_cast<std::int64_t>(rng() % kE18), hi = lo + static_cast<std::int64_t>(rng() % static_cast<std::uint64_t>(kE18 - lo + 1));
        sink += static_cast<std::uint64_t>(dp::count_digit_sum(lo, hi, static_cast<int>(rng() % 100)));
        sink += static_cast<std::uint64_t>(dp::count_no_adjacent_equal(lo, hi));
        sink += static_cast<std::uint64_t>(dp::count_digit_occurrences(lo, hi, static_cast<int>(rng() % 10)));
    }
    CHECK(sink > 0);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
