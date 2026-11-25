// Reference solution for problem 42.
#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

namespace idioms {

inline std::optional<std::int64_t> checked_add(std::int64_t a, std::int64_t b) {
    std::int64_t r;
    if (__builtin_add_overflow(a, b, &r)) return std::nullopt;
    return r;
}

inline std::optional<std::int64_t> checked_mul(std::int64_t a, std::int64_t b) {
    std::int64_t r;
    if (__builtin_mul_overflow(a, b, &r)) return std::nullopt;
    return r;
}

inline std::int64_t saturating_add(std::int64_t a, std::int64_t b) {
    if (auto r = checked_add(a, b)) return *r;
    return b > 0 ? std::numeric_limits<std::int64_t>::max() : std::numeric_limits<std::int64_t>::min();
}

inline std::int64_t notional(std::int64_t price_ticks, std::int64_t qty) {
    if (auto r = checked_mul(price_ticks, qty)) return *r;
    throw std::overflow_error("notional overflows int64");
}

inline std::int64_t mid(std::int64_t a, std::int64_t b) {
    const auto ua = static_cast<std::uint64_t>(a);
    const auto ub = static_cast<std::uint64_t>(b);
    if (a <= b) return static_cast<std::int64_t>(ua + (ub - ua) / 2);
    return static_cast<std::int64_t>(ua - (ua - ub) / 2);
}

inline std::uint64_t safe_abs(std::int64_t x) {
    return x < 0 ? 0 - static_cast<std::uint64_t>(x) : static_cast<std::uint64_t>(x);
}

inline bool seq_after(std::uint32_t a, std::uint32_t b) { return static_cast<std::int32_t>(a - b) > 0; }

inline long index_before_last(const std::vector<int>& v) {
    return v.size() < 2 ? -1 : static_cast<long>(v.size()) - 2;
}

}  // namespace idioms
