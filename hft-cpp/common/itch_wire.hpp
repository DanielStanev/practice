// Reference encoder for the ITCH-like wire format of problem 15.
//
// Tests (15, 16, 19, 20) use this to build byte streams independently of
// your own encoder. It is deliberately simple and unoptimised. Do not
// include it from your solutions.
//
// Wire format (all integers big-endian):
//   uint16 length   number of bytes that follow (type byte + body)
//   char   type
//   body           see problems/15_itch_parser/README.md
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>

namespace wire {

struct Writer {
    std::vector<std::byte> bytes;

    void u8(std::uint8_t v) { bytes.push_back(std::byte{v}); }
    void ch(char c) { u8(static_cast<std::uint8_t>(c)); }
    void u16(std::uint16_t v) {
        u8(std::uint8_t(v >> 8));
        u8(std::uint8_t(v));
    }
    void u32(std::uint32_t v) {
        for (int s = 24; s >= 0; s -= 8) u8(std::uint8_t(v >> s));
    }
    void u64(std::uint64_t v) {
        for (int s = 56; s >= 0; s -= 8) u8(std::uint8_t(v >> s));
    }
    void stock(std::string_view s) {
        for (std::size_t i = 0; i < 8; ++i) ch(i < s.size() ? s[i] : ' ');
    }
    void append(const std::vector<std::byte>& other) {
        bytes.insert(bytes.end(), other.begin(), other.end());
    }

    void add_order(std::uint64_t ts, std::uint64_t ref, char side, std::uint32_t shares,
                   std::string_view stock_name, std::uint32_t price) {
        u16(34);
        ch('A');
        u64(ts);
        u64(ref);
        ch(side);
        u32(shares);
        stock(stock_name);
        u32(price);
    }
    void executed(std::uint64_t ts, std::uint64_t ref, std::uint32_t shares, std::uint64_t match) {
        u16(29);
        ch('E');
        u64(ts);
        u64(ref);
        u32(shares);
        u64(match);
    }
    void cancel(std::uint64_t ts, std::uint64_t ref, std::uint32_t shares) {
        u16(21);
        ch('X');
        u64(ts);
        u64(ref);
        u32(shares);
    }
    void del(std::uint64_t ts, std::uint64_t ref) {
        u16(17);
        ch('D');
        u64(ts);
        u64(ref);
    }
    void replace(std::uint64_t ts, std::uint64_t orig_ref, std::uint64_t new_ref,
                 std::uint32_t shares, std::uint32_t price) {
        u16(33);
        ch('U');
        u64(ts);
        u64(orig_ref);
        u64(new_ref);
        u32(shares);
        u32(price);
    }
    // A message type the parser does not know; must be skipped.
    void unknown(char type, std::size_t body_len) {
        u16(std::uint16_t(1 + body_len));
        ch(type);
        for (std::size_t i = 0; i < body_len; ++i) u8(0xAB);
    }
};

// MoldUDP64-style packet used by problems 16, 19 and 20:
//   char   session[10]
//   uint64 sequence number of the first message
//   uint16 message count (0 = heartbeat)
//   then `count` times: uint16 message length, message bytes (an ITCH
//   message *without* its own uint16 length prefix: type byte + body)
struct Packet {
    std::vector<std::byte> bytes;

    Packet(std::string_view session, std::uint64_t seq, std::uint16_t count) {
        Writer w;
        for (std::size_t i = 0; i < 10; ++i) w.ch(i < session.size() ? session[i] : ' ');
        w.u64(seq);
        w.u16(count);
        bytes = std::move(w.bytes);
    }

    // Appends one ITCH message produced by a Writer holding exactly one
    // message (length prefix included, as Writer produces it).
    void add_itch(const Writer& one_message) {
        bytes.insert(bytes.end(), one_message.bytes.begin(), one_message.bytes.end());
    }

    // Appends an opaque message payload with its uint16 length.
    void add_raw(const std::vector<std::byte>& payload) {
        Writer w;
        w.u16(std::uint16_t(payload.size()));
        bytes.insert(bytes.end(), w.bytes.begin(), w.bytes.end());
        bytes.insert(bytes.end(), payload.begin(), payload.end());
    }
};

}  // namespace wire
