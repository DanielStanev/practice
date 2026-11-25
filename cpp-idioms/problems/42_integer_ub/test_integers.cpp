#include "integers.hpp"

#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
constexpr std::int64_t kMax = std::numeric_limits<std::int64_t>::max();
constexpr std::int64_t kMin = std::numeric_limits<std::int64_t>::min();
}  // namespace

TEST(checked_add_values) {
    CHECK(checked_add(1, 2) == std::optional<std::int64_t>(3));
    CHECK(checked_add(kMax, 0) == std::optional<std::int64_t>(kMax));
    CHECK_FALSE(checked_add(kMax, 1).has_value());
    CHECK_FALSE(checked_add(kMin, -1).has_value());
    CHECK(checked_add(kMax, kMin) == std::optional<std::int64_t>(-1));
}

TEST(checked_mul_values) {
    CHECK(checked_mul(3, -4) == std::optional<std::int64_t>(-12));
    CHECK_FALSE(checked_mul(kMax, 2).has_value());
    CHECK_FALSE(checked_mul(kMin, -1).has_value());
    CHECK(checked_mul(kMin, 1) == std::optional<std::int64_t>(kMin));
    CHECK_FALSE(checked_mul(std::int64_t{1} << 32, std::int64_t{1} << 31).has_value());
    CHECK(checked_mul(std::int64_t{1} << 31, std::int64_t{1} << 31) == std::optional<std::int64_t>(std::int64_t{1} << 62));
}

TEST(saturating_add_values) {
    CHECK_EQ(saturating_add(kMax, 5), kMax);
    CHECK_EQ(saturating_add(kMin, -5), kMin);
    CHECK_EQ(saturating_add(-5, 3), std::int64_t{-2});
}

TEST(notional_values) {
    CHECK_EQ(notional(18950, 1000), std::int64_t{18950000});
    CHECK_EQ(notional(-5, 7), std::int64_t{-35});
    CHECK_THROWS_AS(notional(kMax / 2, 3), std::overflow_error);
}

TEST(mid_no_overflow) {
    CHECK_EQ(mid(2, 4), std::int64_t{3});
    CHECK_EQ(mid(2, 5), std::int64_t{3});   // towards a
    CHECK_EQ(mid(5, 2), std::int64_t{4});   // towards a
    CHECK_EQ(mid(-3, 3), std::int64_t{0});
    CHECK_EQ(mid(kMax, kMax - 2), kMax - 1);
    CHECK_EQ(mid(kMin, kMax), std::int64_t{-1});
    CHECK_EQ(mid(kMax, kMin), std::int64_t{0});
    for (std::int64_t a : {kMin, kMin + 1, std::int64_t{-7}, std::int64_t{0}, std::int64_t{9}, kMax - 1, kMax})
        for (std::int64_t b : {kMin, std::int64_t{-1}, std::int64_t{4}, kMax})
            CHECK_EQ(mid(a, b), std::midpoint(a, b));
}

TEST(safe_abs_values) {
    CHECK_EQ(safe_abs(-5), std::uint64_t{5});
    CHECK_EQ(safe_abs(5), std::uint64_t{5});
    CHECK_EQ(safe_abs(kMin), std::uint64_t{1} << 63);
    CHECK_EQ(safe_abs(kMax), static_cast<std::uint64_t>(kMax));
}

TEST(sequence_numbers_wrap) {
    CHECK(seq_after(2, 1));
    CHECK_FALSE(seq_after(1, 2));
    CHECK_FALSE(seq_after(7, 7));
    CHECK(seq_after(5, 4294967290u));   // after the wrap
    CHECK_FALSE(seq_after(4294967290u, 5));
    CHECK(seq_after(0, 4294967295u));
}

TEST(index_before_last_values) {
    CHECK_EQ(index_before_last({1, 2, 3}), 1L);
    CHECK_EQ(index_before_last({1, 2}), 0L);
    CHECK_EQ(index_before_last({1}), -1L);
    CHECK_EQ(index_before_last({}), -1L);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
