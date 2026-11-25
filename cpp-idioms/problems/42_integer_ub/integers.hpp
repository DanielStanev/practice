// Problem 42: integer arithmetic without undefined behaviour.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

namespace idioms {

inline std::optional<std::int64_t> checked_add(std::int64_t a, std::int64_t b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: checked_add");
}

inline std::optional<std::int64_t> checked_mul(std::int64_t a, std::int64_t b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: checked_mul");
}

inline std::int64_t saturating_add(std::int64_t a, std::int64_t b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: saturating_add");
}

inline std::int64_t notional(std::int64_t price_ticks, std::int64_t qty) {
    (void)price_ticks;
    (void)qty;
    throw std::logic_error("TODO: notional");
}

inline std::int64_t mid(std::int64_t a, std::int64_t b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: mid");
}

inline std::uint64_t safe_abs(std::int64_t x) {
    (void)x;
    throw std::logic_error("TODO: safe_abs");
}

inline bool seq_after(std::uint32_t a, std::uint32_t b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: seq_after");
}

inline long index_before_last(const std::vector<int>& v) {
    (void)v;
    throw std::logic_error("TODO: index_before_last");
}

}  // namespace idioms
