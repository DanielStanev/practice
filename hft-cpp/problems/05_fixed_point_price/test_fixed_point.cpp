#include "fixed_point.hpp"

#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <string>

#include "testing.hpp"

using hft::Fixed;
using hft::Price;
using hft::Rounding;

namespace {

template <typename A, typename B>
concept Multipliable = requires(A a, B b) { a * b; };

Price P(std::string_view s) {
    auto p = Price::parse(s);
    if (!p) throw std::runtime_error("test helper: could not parse " + std::string(s));
    return *p;
}

constexpr std::int64_t kMax = std::numeric_limits<std::int64_t>::max();
constexpr std::int64_t kMin = std::numeric_limits<std::int64_t>::min();

}  // namespace

TEST(scale_and_raw) {
    CHECK_EQ(Price::kScale, 10000);
    CHECK_EQ(Fixed<0>::kScale, 1);
    CHECK_EQ(Fixed<9>::kScale, 1000000000);
    constexpr Price p = Price::from_raw(1234567);
    CHECK_EQ(p.raw(), 1234567);
    constexpr Price q = Price::from_integer(12);
    CHECK_EQ(q.raw(), 120000);
    CHECK_EQ(Price().raw(), 0);
}

TEST(parse_valid) {
    CHECK_EQ(P("123.45").raw(), 1234500);
    CHECK_EQ(P("123.4567").raw(), 1234567);
    CHECK_EQ(P("7").raw(), 70000);
    CHECK_EQ(P("0.0001").raw(), 1);
    CHECK_EQ(P("-0.5").raw(), -5000);
    CHECK_EQ(P("+3.1").raw(), 31000);
    CHECK_EQ(P("000012.3000").raw(), 123000);
    CHECK_EQ(P("-0").raw(), 0);
}

TEST(parse_is_constexpr) {
    constexpr auto p = Price::parse("99.99");
    CHECK(p.has_value());
    CHECK_EQ(p->raw(), 999900);
}

TEST(parse_rejects_garbage) {
    for (std::string_view bad : {"", "-", "+", ".", ".5", "1.", "1.2.3", "12a", "1,5", " 1",
                                 "1 ", "--1", "1e5", "0x10", "1.23456" /* too many decimals */}) {
        if (Price::parse(bad).has_value())
            ::testing::fail(__FILE__, __LINE__, "parsed invalid input '" + std::string(bad) + "'");
    }
}

TEST(parse_rejects_overflow) {
    // Fixed<4> covers about +/- 922 trillion.
    CHECK(Price::parse("922337203685477.5807").has_value());
    CHECK_EQ(Price::parse("922337203685477.5807")->raw(), kMax);
    CHECK_EQ(Price::parse("-922337203685477.5808")->raw(), kMin);
    CHECK_FALSE(Price::parse("922337203685477.5808").has_value());
    CHECK_FALSE(Price::parse("1000000000000000").has_value());
    CHECK_FALSE(Price::parse("99999999999999999999999").has_value());
}

TEST(to_string_formats_fixed_decimals) {
    CHECK_EQ(P("123.45").to_string(), std::string("123.4500"));
    CHECK_EQ(P("-0.5").to_string(), std::string("-0.5000"));
    CHECK_EQ(Price().to_string(), std::string("0.0000"));
    CHECK_EQ(Price::from_raw(-1).to_string(), std::string("-0.0001"));
    CHECK_EQ(Fixed<0>::from_raw(-42).to_string(), std::string("-42"));
    CHECK_EQ(Fixed<2>::from_raw(5).to_string(), std::string("0.05"));
    CHECK_EQ(Price::from_raw(kMin).to_string(), std::string("-922337203685477.5808"));
}

TEST(round_trip_random) {
    std::mt19937_64 rng(42);
    for (int i = 0; i < 100000; ++i) {
        auto raw = static_cast<std::int64_t>(rng());
        Price p = Price::from_raw(raw);
        auto back = Price::parse(p.to_string());
        CHECK(back.has_value());
        CHECK_EQ(back->raw(), raw);
    }
}

TEST(no_floating_point_drift) {
    // 0.1 + 0.2 != 0.3 in binary floating point. In fixed point it is exact.
    CHECK(P("0.1") + P("0.2") == P("0.3"));
    Price sum;
    for (int i = 0; i < 1000000; ++i) sum += P("0.0001");
    CHECK(sum == P("100"));
}

TEST(arithmetic) {
    constexpr Price a = Price::from_raw(15000);  // 1.5
    constexpr Price b = Price::from_raw(2500);   // 0.25
    constexpr Price c = a - b;
    CHECK_EQ(c.raw(), 12500);
    CHECK_EQ((a + b).raw(), 17500);
    CHECK_EQ((-a).raw(), -15000);
    Price d = a;
    d -= b;
    d += b;
    CHECK(d == a);
}

TEST(multiply_by_quantity) {
    Price px = P("101.25");
    CHECK(px * 100 == P("10125"));
    CHECK(3 * px == P("303.75"));
    CHECK(px * std::uint32_t{2} == P("202.5"));
}

TEST(type_safety_with_concepts) {
    CHECK_TRAIT(Multipliable<Price, int>);
    CHECK_TRAIT(Multipliable<Price, std::uint64_t>);
    CHECK_TRAIT(Multipliable<long, Price>);
    // Price * Price has no meaning (units would be price^2), and a double
    // would silently reintroduce floating point.
    CHECK_TRAIT(!Multipliable<Price, Price>);
    CHECK_TRAIT(!Multipliable<Price, double>);
    CHECK_TRAIT(!Multipliable<Price, float>);
    // Different scales do not mix implicitly.
    CHECK_TRAIT(!std::is_convertible_v<Fixed<2>, Fixed<4>>);
    // Raw integers do not silently become prices.
    CHECK_TRAIT(!std::is_convertible_v<std::int64_t, Price>);
}

TEST(comparisons) {
    CHECK(P("1.0001") > P("1"));
    CHECK(P("-2") < P("-1.9999"));
    CHECK(P("5") == P("5.0000"));
    CHECK(P("5") != P("5.0001"));
    CHECK(P("3") >= P("3"));
}

TEST(checked_arithmetic) {
    Price big = Price::from_raw(kMax - 5);
    CHECK_FALSE(big.checked_add(Price::from_raw(6)).has_value());
    CHECK_EQ(big.checked_add(Price::from_raw(5))->raw(), kMax);
    CHECK_FALSE(Price::from_raw(kMin).checked_add(Price::from_raw(-1)).has_value());
    CHECK_FALSE(P("1000000").checked_mul(1000000000).has_value());
    CHECK(P("1000000").checked_mul(1000).has_value());
    CHECK_EQ(P("2.5").checked_mul(-4)->raw(), -100000);
}

TEST(round_to_tick) {
    Price tick = P("0.05");
    CHECK(P("10.12").round_to_tick(tick, Rounding::Down) == P("10.10"));
    CHECK(P("10.12").round_to_tick(tick, Rounding::Up) == P("10.15"));
    CHECK(P("10.12").round_to_tick(tick, Rounding::Nearest) == P("10.10"));
    CHECK(P("10.13").round_to_tick(tick, Rounding::Nearest) == P("10.15"));
    CHECK(P("10.15").round_to_tick(tick, Rounding::Up) == P("10.15"));  // already on tick
    // Down means toward negative infinity, Up toward positive infinity.
    CHECK(P("-10.12").round_to_tick(tick, Rounding::Down) == P("-10.15"));
    CHECK(P("-10.12").round_to_tick(tick, Rounding::Up) == P("-10.10"));
    // Ties round away from zero.
    CHECK(P("10.125").round_to_tick(tick, Rounding::Nearest) == P("10.15"));
    CHECK(P("-10.125").round_to_tick(tick, Rounding::Nearest) == P("-10.15"));
    constexpr Price r = Price::from_raw(101).round_to_tick(Price::from_raw(10), Rounding::Up);
    CHECK_EQ(r.raw(), 110);
}

TEST(to_double_is_close) {
    CHECK_NEAR(P("123.4567").to_double(), 123.4567, 1e-9);
    CHECK_NEAR(P("-0.0001").to_double(), -0.0001, 1e-12);
}

TEST(zero_overhead) {
    CHECK_EQ(sizeof(Price), sizeof(std::int64_t));
    CHECK_TRAIT(std::is_trivially_copyable_v<Price>);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
