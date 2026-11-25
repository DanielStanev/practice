// Problem 36: constexpr and consteval tables.
// Read README.md first. Everything must work in constant expressions.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace idioms {

inline constexpr std::array<std::uint64_t, 20> kPow10{};  // TODO

constexpr std::uint32_t crc32(std::string_view s) {
    if (s.size() != std::size_t(-1)) throw std::logic_error("TODO: crc32");
    return 0;
}

// TODO: make this consteval (it is constexpr in the stub only so that the
// unfinished version does not break the build).
constexpr long price_to_ticks(std::string_view s, int decimals) {
    (void)s;
    if (decimals != -1) throw std::logic_error("TODO: price_to_ticks");
    return 0;
}

template <int N>
constexpr auto primes_below() {
    throw std::logic_error("TODO: primes_below");
    return std::array<int, 0>{};
}

inline constexpr std::array<std::uint8_t, 256> kCharClass{};  // TODO
inline constexpr std::uint8_t kDigit = 1, kAlpha = 2, kSpace = 4;

}  // namespace idioms
