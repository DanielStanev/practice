// Problem 15: zero-copy parser for an ITCH-like binary market data protocol
// Read README.md first (it has the byte-level spec). Implement everything
// marked TODO. You may add helpers, but keep the public interface.
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

// Decoded messages: native byte order, natural alignment. These are NOT the
// wire layout; never reinterpret_cast wire bytes to them.
struct AddOrder {
    std::uint64_t timestamp;
    std::uint64_t order_ref;
    Side side;
    std::uint32_t shares;
    std::array<char, 8> stock;  // space padded, not NUL terminated
    std::uint32_t price;        // 4 implied decimals
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

// Value of the length field (type byte + body) for each message type.
inline constexpr std::size_t kAddOrderLen = 34;
inline constexpr std::size_t kOrderExecutedLen = 29;
inline constexpr std::size_t kOrderCancelLen = 21;
inline constexpr std::size_t kOrderDeleteLen = 17;
inline constexpr std::size_t kOrderReplaceLen = 33;

// A handler has on_add / on_execute / on_cancel / on_delete / on_replace
// taking the decoded message by const reference.
template <typename H>
concept Handler = true;  // TODO

enum class ParseError { None, BadLength, BadSide };

struct ParseResult {
    std::size_t consumed = 0;  // bytes of complete messages processed
    std::size_t messages = 0;  // complete messages processed (incl. unknown types)
    std::size_t unknown = 0;   // messages of unknown type that were skipped
    ParseError error = ParseError::None;
};

// Reads a big-endian unsigned integer from a possibly unaligned address.
template <std::unsigned_integral T>
inline T load_be(const std::byte* p) noexcept {
    (void)p;
    return 0;  // TODO
}

// Writes a big-endian unsigned integer to a possibly unaligned address.
template <std::unsigned_integral T>
inline void store_be(std::byte* p, T v) noexcept {
    (void)p;
    (void)v;
    // TODO
}

// Parses as many complete messages from buf as possible, calling the
// matching handler function for each known message. See README for the
// exact rules on incomplete data, unknown types and errors.
template <Handler H>
ParseResult parse(std::span<const std::byte> buf, H& h) {
    (void)buf;
    (void)h;
    return {};  // TODO
}

// Encoders: write one framed message (length prefix included) into out.
// Return the number of bytes written, or 0 if out is too small.
inline std::size_t encode(const AddOrder& m, std::span<std::byte> out) noexcept {
    (void)m;
    (void)out;
    return 0;  // TODO
}
inline std::size_t encode(const OrderExecuted& m, std::span<std::byte> out) noexcept {
    (void)m;
    (void)out;
    return 0;  // TODO
}
inline std::size_t encode(const OrderCancel& m, std::span<std::byte> out) noexcept {
    (void)m;
    (void)out;
    return 0;  // TODO
}
inline std::size_t encode(const OrderDelete& m, std::span<std::byte> out) noexcept {
    (void)m;
    (void)out;
    return 0;  // TODO
}
inline std::size_t encode(const OrderReplace& m, std::span<std::byte> out) noexcept {
    (void)m;
    (void)out;
    return 0;  // TODO
}

}  // namespace hft::itch
