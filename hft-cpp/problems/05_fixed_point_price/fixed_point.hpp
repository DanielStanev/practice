// Problem 05: fixed-point Price type
// Read README.md first. Implement everything marked TODO.
// Placeholders compile (and are constexpr) but return wrong answers.
// Do not use floating point anywhere except to_double().
#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <limits>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace hft {

enum class Rounding {
    Down,     // toward negative infinity
    Up,       // toward positive infinity
    Nearest,  // nearest tick, ties away from zero
};

// A decimal number stored as an int64 count of 10^-Decimals units.
// Fixed<4>::from_raw(1234567) represents 123.4567.
template <int Decimals>
class Fixed {
    static_assert(Decimals >= 0 && Decimals <= 9, "Decimals must be in [0, 9]");

public:
    using raw_type = std::int64_t;
    static constexpr std::int64_t kScale = 1;  // TODO: 10^Decimals, computed at compile time
    static constexpr int kDecimals = Decimals;

    constexpr Fixed() = default;

    static constexpr Fixed from_raw(std::int64_t raw) {
        (void)raw;
        return Fixed{};  // TODO
    }

    // TODO: constrain to integral types.
    template <typename I>
    static constexpr Fixed from_integer(I v) {
        (void)v;
        return Fixed{};  // TODO
    }

    constexpr std::int64_t raw() const { return raw_; }

    // Parses [+-]digits[.digits] with at most Decimals fractional digits.
    // Returns nullopt for anything else, including values that overflow.
    // Must be constexpr. Must not use floating point.
    static constexpr std::optional<Fixed> parse(std::string_view s) {
        (void)s;
        return std::nullopt;  // TODO
    }

    // Always prints exactly Decimals fractional digits ("12.3400", "-0.0001").
    // Fixed<0> prints no decimal point.
    std::string to_string() const {
        return "TODO";  // TODO
    }

    constexpr double to_double() const {
        return 0.0;  // TODO
    }

    constexpr Fixed operator+(Fixed o) const {
        (void)o;
        return Fixed{};  // TODO
    }
    constexpr Fixed operator-(Fixed o) const {
        (void)o;
        return Fixed{};  // TODO
    }
    constexpr Fixed operator-() const {
        return Fixed{};  // TODO
    }
    constexpr Fixed& operator+=(Fixed o) {
        (void)o;
        return *this;  // TODO
    }
    constexpr Fixed& operator-=(Fixed o) {
        (void)o;
        return *this;  // TODO
    }

    // Price * quantity. TODO: only integral quantities may be accepted.
    template <typename I>
    constexpr Fixed operator*(I qty) const {
        (void)qty;
        return Fixed{};  // TODO
    }
    template <typename I>
    friend constexpr Fixed operator*(I qty, Fixed f) {
        return f * qty;
    }

    // nullopt on signed overflow.
    constexpr std::optional<Fixed> checked_add(Fixed o) const {
        (void)o;
        return std::nullopt;  // TODO
    }

    template <std::integral I>
    constexpr std::optional<Fixed> checked_mul(I qty) const {
        (void)qty;
        return std::nullopt;  // TODO
    }

    // Rounds to a multiple of tick (tick > 0).
    constexpr Fixed round_to_tick(Fixed tick, Rounding mode) const {
        (void)tick;
        (void)mode;
        return *this;  // TODO
    }

    // TODO: comparisons. (Hint: one line in C++20.)
    constexpr bool operator==(const Fixed&) const { return false; }
    constexpr bool operator<(const Fixed&) const { return false; }
    constexpr bool operator>(const Fixed&) const { return false; }
    constexpr bool operator<=(const Fixed&) const { return false; }
    constexpr bool operator>=(const Fixed&) const { return false; }

    friend std::ostream& operator<<(std::ostream& os, const Fixed& f) {
        return os << f.to_string();
    }

private:
    std::int64_t raw_ = 0;
};

using Price = Fixed<4>;

}  // namespace hft
