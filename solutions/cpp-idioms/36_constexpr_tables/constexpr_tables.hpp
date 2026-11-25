// Reference solution for problem 36.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace idioms {

namespace detail {
constexpr std::array<std::uint64_t, 20> make_pow10() {
    std::array<std::uint64_t, 20> t{};
    std::uint64_t v = 1;
    for (auto& x : t) {
        x = v;
        if (&x != &t.back()) v *= 10;
    }
    return t;
}
constexpr std::array<std::uint32_t, 256> make_crc_table() {
    std::array<std::uint32_t, 256> t{};
    for (std::uint32_t i = 0; i < 256; ++i) {
        std::uint32_t c = i;
        for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        t[i] = c;
    }
    return t;
}
inline constexpr auto kCrcTable = make_crc_table();

constexpr bool is_prime(int n) {
    if (n < 2) return false;
    for (int d = 2; d * d <= n; ++d)
        if (n % d == 0) return false;
    return true;
}
constexpr std::size_t count_primes_below(int n) {
    std::size_t c = 0;
    for (int i = 2; i < n; ++i) c += is_prime(i);
    return c;
}
constexpr std::array<std::uint8_t, 256> make_char_class() {
    std::array<std::uint8_t, 256> t{};
    for (int c = '0'; c <= '9'; ++c) t[c] |= 1;
    for (int c = 'a'; c <= 'z'; ++c) t[c] |= 2;
    for (int c = 'A'; c <= 'Z'; ++c) t[c] |= 2;
    for (char c : {' ', '\t', '\n', '\r'}) t[static_cast<unsigned char>(c)] |= 4;
    return t;
}
}  // namespace detail

inline constexpr std::array<std::uint64_t, 20> kPow10 = detail::make_pow10();

constexpr std::uint32_t crc32(std::string_view s) {
    std::uint32_t crc = 0xFFFFFFFFu;
    for (char ch : s) crc = detail::kCrcTable[(crc ^ static_cast<unsigned char>(ch)) & 0xFFu] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

consteval long price_to_ticks(std::string_view s, int decimals) {
    if (s.empty()) throw "empty price";
    long whole = 0, frac = 0;
    int frac_digits = 0;
    bool seen_dot = false;
    for (char c : s) {
        if (c == '.') {
            if (seen_dot) throw "two decimal points";
            seen_dot = true;
        } else if (c >= '0' && c <= '9') {
            if (seen_dot) {
                if (++frac_digits > decimals) throw "too many decimals";
                frac = frac * 10 + (c - '0');
            } else {
                whole = whole * 10 + (c - '0');
            }
        } else {
            throw "not a digit";
        }
    }
    for (int i = frac_digits; i < decimals; ++i) frac *= 10;
    return whole * static_cast<long>(kPow10[static_cast<std::size_t>(decimals)]) + frac;
}

template <int N>
constexpr auto primes_below() {
    std::array<int, detail::count_primes_below(N)> out{};
    std::size_t k = 0;
    for (int i = 2; i < N; ++i)
        if (detail::is_prime(i)) out[k++] = i;
    return out;
}

inline constexpr std::array<std::uint8_t, 256> kCharClass = detail::make_char_class();
inline constexpr std::uint8_t kDigit = 1, kAlpha = 2, kSpace = 4;

}  // namespace idioms
