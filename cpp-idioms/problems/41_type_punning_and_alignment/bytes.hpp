// Problem 41: bytes, type punning, alignment, strict aliasing.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace idioms {

template <class T>
T load(const std::byte* p) {
    (void)p;
    throw std::logic_error("TODO: load");
}

template <class T>
void store(std::byte* p, T v) {
    (void)p;
    (void)v;
    throw std::logic_error("TODO: store");
}

template <std::unsigned_integral T>
constexpr T byteswap(T x) {
    (void)x;
    throw std::logic_error("TODO: byteswap");
    return 0;
}

template <std::unsigned_integral T>
T load_be(const std::byte* p) {
    (void)p;
    throw std::logic_error("TODO: load_be");
}

template <std::unsigned_integral T>
T load_le(const std::byte* p) {
    (void)p;
    throw std::logic_error("TODO: load_le");
}

constexpr std::uint64_t double_bits(double d) {
    if (d == d) throw std::logic_error("TODO: double_bits");
    return 0;
}

constexpr double bits_double(std::uint64_t b) {
    if (b != 1) throw std::logic_error("TODO: bits_double");
    return 0;
}

struct Header {
    std::uint16_t length;
    std::uint8_t type;
    std::uint64_t seq;
    std::int32_t price_ticks;
};

inline Header parse_header(const std::byte* buf, std::size_t offset) {
    (void)buf;
    (void)offset;
    throw std::logic_error("TODO: parse_header");
}

}  // namespace idioms
