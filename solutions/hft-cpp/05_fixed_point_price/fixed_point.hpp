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

enum class Rounding { Down, Up, Nearest };

template <int Decimals>
class Fixed {
    static_assert(Decimals >= 0 && Decimals <= 9, "Decimals must be in [0, 9]");

    static constexpr std::int64_t compute_scale() {
        std::int64_t s = 1;
        for (int i = 0; i < Decimals; ++i) s *= 10;
        return s;
    }

public:
    using raw_type = std::int64_t;
    static constexpr std::int64_t kScale = compute_scale();
    static constexpr int kDecimals = Decimals;

    constexpr Fixed() = default;

    static constexpr Fixed from_raw(std::int64_t raw) {
        Fixed f;
        f.raw_ = raw;
        return f;
    }

    template <std::integral I>
    static constexpr Fixed from_integer(I v) {
        return from_raw(static_cast<std::int64_t>(v) * kScale);
    }

    constexpr std::int64_t raw() const { return raw_; }

    static constexpr std::optional<Fixed> parse(std::string_view s) {
        if (s.empty()) return std::nullopt;
        bool negative = false;
        std::size_t i = 0;
        if (s[0] == '-' || s[0] == '+') {
            negative = s[0] == '-';
            ++i;
        }
        // Accumulate as a negative number so INT64_MIN is representable.
        std::int64_t acc = 0;
        int int_digits = 0;
        for (; i < s.size() && s[i] != '.'; ++i) {
            char c = s[i];
            if (c < '0' || c > '9') return std::nullopt;
            if (__builtin_mul_overflow(acc, 10, &acc)) return std::nullopt;
            if (__builtin_sub_overflow(acc, c - '0', &acc)) return std::nullopt;
            ++int_digits;
        }
        if (int_digits == 0) return std::nullopt;
        int frac_digits = 0;
        if (i < s.size()) {
            ++i;  // '.'
            if (i == s.size()) return std::nullopt;
            for (; i < s.size(); ++i) {
                char c = s[i];
                if (c < '0' || c > '9') return std::nullopt;
                if (++frac_digits > Decimals) return std::nullopt;
                if (__builtin_mul_overflow(acc, 10, &acc)) return std::nullopt;
                if (__builtin_sub_overflow(acc, c - '0', &acc)) return std::nullopt;
            }
        }
        for (; frac_digits < Decimals; ++frac_digits)
            if (__builtin_mul_overflow(acc, 10, &acc)) return std::nullopt;
        if (!negative) {
            if (acc == std::numeric_limits<std::int64_t>::min()) return std::nullopt;
            acc = -acc;
        }
        return from_raw(acc);
    }

    std::string to_string() const {
        // Work with unsigned magnitude so INT64_MIN works.
        std::uint64_t mag = raw_ < 0 ? 0 - static_cast<std::uint64_t>(raw_)
                                     : static_cast<std::uint64_t>(raw_);
        std::uint64_t ip = mag / static_cast<std::uint64_t>(kScale);
        std::uint64_t fp = mag % static_cast<std::uint64_t>(kScale);
        std::string out;
        if (raw_ < 0) out += '-';
        out += std::to_string(ip);
        if constexpr (Decimals > 0) {
            char frac[Decimals];
            for (int k = Decimals - 1; k >= 0; --k) {
                frac[k] = static_cast<char>('0' + fp % 10);
                fp /= 10;
            }
            out += '.';
            out.append(frac, Decimals);
        }
        return out;
    }

    constexpr double to_double() const {
        return static_cast<double>(raw_) / static_cast<double>(kScale);
    }

    constexpr Fixed operator+(Fixed o) const { return from_raw(raw_ + o.raw_); }
    constexpr Fixed operator-(Fixed o) const { return from_raw(raw_ - o.raw_); }
    constexpr Fixed operator-() const { return from_raw(-raw_); }
    constexpr Fixed& operator+=(Fixed o) {
        raw_ += o.raw_;
        return *this;
    }
    constexpr Fixed& operator-=(Fixed o) {
        raw_ -= o.raw_;
        return *this;
    }

    template <std::integral I>
    constexpr Fixed operator*(I qty) const {
        return from_raw(raw_ * static_cast<std::int64_t>(qty));
    }
    template <std::integral I>
    friend constexpr Fixed operator*(I qty, Fixed f) {
        return f * qty;
    }

    constexpr std::optional<Fixed> checked_add(Fixed o) const {
        std::int64_t r;
        if (__builtin_add_overflow(raw_, o.raw_, &r)) return std::nullopt;
        return from_raw(r);
    }

    template <std::integral I>
    constexpr std::optional<Fixed> checked_mul(I qty) const {
        std::int64_t r;
        if (__builtin_mul_overflow(raw_, static_cast<std::int64_t>(qty), &r)) return std::nullopt;
        return from_raw(r);
    }

    constexpr Fixed round_to_tick(Fixed tick, Rounding mode) const {
        const std::int64_t t = tick.raw_;
        std::int64_t q = raw_ / t;
        std::int64_t r = raw_ % t;
        if (r == 0) return *this;
        // C++ division truncates toward zero; fix up to floor.
        if (r < 0) {
            --q;
            r += t;
        }
        switch (mode) {
            case Rounding::Down:
                break;
            case Rounding::Up:
                ++q;
                break;
            case Rounding::Nearest:
                // Ties go away from zero.
                if (2 * r > t || (2 * r == t && raw_ > 0)) ++q;
                break;
        }
        return from_raw(q * t);
    }

    constexpr auto operator<=>(const Fixed&) const = default;
    constexpr bool operator==(const Fixed&) const = default;

    friend std::ostream& operator<<(std::ostream& os, const Fixed& f) {
        return os << f.to_string();
    }

private:
    std::int64_t raw_ = 0;
};

using Price = Fixed<4>;

}  // namespace hft
