#pragma once

#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>

namespace hft::itch {

enum class Side : char { Buy = 'B', Sell = 'S' };

struct AddOrder {
    std::uint64_t timestamp;
    std::uint64_t order_ref;
    Side side;
    std::uint32_t shares;
    std::array<char, 8> stock;
    std::uint32_t price;
};
struct OrderExecuted {
    std::uint64_t timestamp;
    std::uint64_t order_ref;
    std::uint32_t executed_shares;
    std::uint64_t match_number;
};
struct OrderCancel {
    std::uint64_t timestamp;
    std::uint64_t order_ref;
    std::uint32_t cancelled_shares;
};
struct OrderDelete {
    std::uint64_t timestamp;
    std::uint64_t order_ref;
};
struct OrderReplace {
    std::uint64_t timestamp;
    std::uint64_t orig_order_ref;
    std::uint64_t new_order_ref;
    std::uint32_t shares;
    std::uint32_t price;
};

inline constexpr std::size_t kAddOrderLen = 34;
inline constexpr std::size_t kOrderExecutedLen = 29;
inline constexpr std::size_t kOrderCancelLen = 21;
inline constexpr std::size_t kOrderDeleteLen = 17;
inline constexpr std::size_t kOrderReplaceLen = 33;

template <typename H>
concept Handler = requires(H& h, const AddOrder& a, const OrderExecuted& e, const OrderCancel& c,
                           const OrderDelete& d, const OrderReplace& r) {
    h.on_add(a);
    h.on_execute(e);
    h.on_cancel(c);
    h.on_delete(d);
    h.on_replace(r);
};

enum class ParseError { None, BadLength, BadSide };

struct ParseResult {
    std::size_t consumed = 0;
    std::size_t messages = 0;
    std::size_t unknown = 0;
    ParseError error = ParseError::None;
};

template <std::unsigned_integral T>
inline T load_be(const std::byte* p) noexcept {
    T v;
    std::memcpy(&v, p, sizeof(T));
    if constexpr (std::endian::native == std::endian::little && sizeof(T) > 1) {
        if constexpr (sizeof(T) == 2) v = static_cast<T>(__builtin_bswap16(v));
        else if constexpr (sizeof(T) == 4) v = static_cast<T>(__builtin_bswap32(v));
        else v = static_cast<T>(__builtin_bswap64(v));
    }
    return v;
}

template <std::unsigned_integral T>
inline void store_be(std::byte* p, T v) noexcept {
    if constexpr (std::endian::native == std::endian::little && sizeof(T) > 1) {
        if constexpr (sizeof(T) == 2) v = static_cast<T>(__builtin_bswap16(v));
        else if constexpr (sizeof(T) == 4) v = static_cast<T>(__builtin_bswap32(v));
        else v = static_cast<T>(__builtin_bswap64(v));
    }
    std::memcpy(p, &v, sizeof(T));
}

template <Handler H>
ParseResult parse(std::span<const std::byte> buf, H& h) {
    ParseResult r;
    const std::byte* p = buf.data();
    std::size_t left = buf.size();
    while (left >= 2) {
        const std::size_t len = load_be<std::uint16_t>(p);
        if (len == 0) {
            r.error = ParseError::BadLength;
            return r;
        }
        if (left < 2 + len) break;  // incomplete: wait for more bytes
        const std::byte* m = p + 2;
        const char type = static_cast<char>(m[0]);
        const std::byte* b = m + 1;
        auto expect = [&](std::size_t n) {
            if (len == n) return true;
            r.error = ParseError::BadLength;
            return false;
        };
        switch (type) {
            case 'A': {
                if (!expect(kAddOrderLen)) return r;
                char side = static_cast<char>(b[16]);
                if (side != 'B' && side != 'S') {
                    r.error = ParseError::BadSide;
                    return r;
                }
                AddOrder a;
                a.timestamp = load_be<std::uint64_t>(b);
                a.order_ref = load_be<std::uint64_t>(b + 8);
                a.side = static_cast<Side>(side);
                a.shares = load_be<std::uint32_t>(b + 17);
                std::memcpy(a.stock.data(), b + 21, 8);
                a.price = load_be<std::uint32_t>(b + 29);
                h.on_add(a);
                break;
            }
            case 'E': {
                if (!expect(kOrderExecutedLen)) return r;
                h.on_execute(OrderExecuted{load_be<std::uint64_t>(b), load_be<std::uint64_t>(b + 8),
                                           load_be<std::uint32_t>(b + 16),
                                           load_be<std::uint64_t>(b + 20)});
                break;
            }
            case 'X': {
                if (!expect(kOrderCancelLen)) return r;
                h.on_cancel(OrderCancel{load_be<std::uint64_t>(b), load_be<std::uint64_t>(b + 8),
                                        load_be<std::uint32_t>(b + 16)});
                break;
            }
            case 'D': {
                if (!expect(kOrderDeleteLen)) return r;
                h.on_delete(OrderDelete{load_be<std::uint64_t>(b), load_be<std::uint64_t>(b + 8)});
                break;
            }
            case 'U': {
                if (!expect(kOrderReplaceLen)) return r;
                h.on_replace(OrderReplace{load_be<std::uint64_t>(b), load_be<std::uint64_t>(b + 8),
                                          load_be<std::uint64_t>(b + 16),
                                          load_be<std::uint32_t>(b + 24),
                                          load_be<std::uint32_t>(b + 28)});
                break;
            }
            default:
                ++r.unknown;
                break;
        }
        ++r.messages;
        p += 2 + len;
        left -= 2 + len;
        r.consumed += 2 + len;
    }
    return r;
}

// Encoders: write one framed message (length prefix included) into out.
// Return bytes written, or 0 if out is too small.
inline std::size_t encode(const AddOrder& a, std::span<std::byte> out) noexcept {
    if (out.size() < 2 + kAddOrderLen) return 0;
    std::byte* p = out.data();
    store_be<std::uint16_t>(p, kAddOrderLen);
    p[2] = std::byte{'A'};
    store_be(p + 3, a.timestamp);
    store_be(p + 11, a.order_ref);
    p[19] = static_cast<std::byte>(a.side);
    store_be(p + 20, a.shares);
    std::memcpy(p + 24, a.stock.data(), 8);
    store_be(p + 32, a.price);
    return 2 + kAddOrderLen;
}

inline std::size_t encode(const OrderExecuted& e, std::span<std::byte> out) noexcept {
    if (out.size() < 2 + kOrderExecutedLen) return 0;
    std::byte* p = out.data();
    store_be<std::uint16_t>(p, kOrderExecutedLen);
    p[2] = std::byte{'E'};
    store_be(p + 3, e.timestamp);
    store_be(p + 11, e.order_ref);
    store_be(p + 19, e.executed_shares);
    store_be(p + 23, e.match_number);
    return 2 + kOrderExecutedLen;
}

inline std::size_t encode(const OrderCancel& c, std::span<std::byte> out) noexcept {
    if (out.size() < 2 + kOrderCancelLen) return 0;
    std::byte* p = out.data();
    store_be<std::uint16_t>(p, kOrderCancelLen);
    p[2] = std::byte{'X'};
    store_be(p + 3, c.timestamp);
    store_be(p + 11, c.order_ref);
    store_be(p + 19, c.cancelled_shares);
    return 2 + kOrderCancelLen;
}

inline std::size_t encode(const OrderDelete& d, std::span<std::byte> out) noexcept {
    if (out.size() < 2 + kOrderDeleteLen) return 0;
    std::byte* p = out.data();
    store_be<std::uint16_t>(p, kOrderDeleteLen);
    p[2] = std::byte{'D'};
    store_be(p + 3, d.timestamp);
    store_be(p + 11, d.order_ref);
    return 2 + kOrderDeleteLen;
}

inline std::size_t encode(const OrderReplace& r, std::span<std::byte> out) noexcept {
    if (out.size() < 2 + kOrderReplaceLen) return 0;
    std::byte* p = out.data();
    store_be<std::uint16_t>(p, kOrderReplaceLen);
    p[2] = std::byte{'U'};
    store_be(p + 3, r.timestamp);
    store_be(p + 11, r.orig_order_ref);
    store_be(p + 19, r.new_order_ref);
    store_be(p + 27, r.shares);
    store_be(p + 31, r.price);
    return 2 + kOrderReplaceLen;
}

}  // namespace hft::itch
