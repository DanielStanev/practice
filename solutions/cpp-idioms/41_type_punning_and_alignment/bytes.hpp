// Reference solution for problem 41.
#pragma once

#include <array>
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
    static_assert(std::is_trivially_copyable_v<T>);
    std::array<std::byte, sizeof(T)> raw;
    std::memcpy(raw.data(), p, sizeof(T));
    return std::bit_cast<T>(raw);
}

template <class T>
void store(std::byte* p, T v) {
    static_assert(std::is_trivially_copyable_v<T>);
    std::memcpy(p, &v, sizeof(T));
}

template <std::unsigned_integral T>
constexpr T byteswap(T x) {
    T r = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        r = static_cast<T>((r << 8) | (x & 0xFF));
        x = static_cast<T>(x >> 8);
    }
    return r;
}

template <std::unsigned_integral T>
T load_be(const std::byte* p) {
    T v = load<T>(p);
    if constexpr (std::endian::native == std::endian::little) v = byteswap(v);
    return v;
}

template <std::unsigned_integral T>
T load_le(const std::byte* p) {
    T v = load<T>(p);
    if constexpr (std::endian::native == std::endian::big) v = byteswap(v);
    return v;
}

constexpr std::uint64_t double_bits(double d) { return std::bit_cast<std::uint64_t>(d); }
constexpr double bits_double(std::uint64_t b) { return std::bit_cast<double>(b); }

struct Header {
    std::uint16_t length;
    std::uint8_t type;
    std::uint64_t seq;
    std::int32_t price_ticks;
};

inline Header parse_header(const std::byte* buf, std::size_t offset) {
    const std::byte* p = buf + offset;
    Header h{};
    h.length = load_be<std::uint16_t>(p);
    h.type = static_cast<std::uint8_t>(p[2]);
    h.seq = load_be<std::uint64_t>(p + 3);
    h.price_ticks = static_cast<std::int32_t>(load_be<std::uint32_t>(p + 11));
    return h;
}

}  // namespace idioms
