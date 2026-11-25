#include "itch_parser.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <random>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "itch_wire.hpp"
#include "testing.hpp"

namespace itch = hft::itch;

namespace {

using Event = std::variant<itch::AddOrder, itch::OrderExecuted, itch::OrderCancel, itch::OrderDelete,
                           itch::OrderReplace>;

struct Recorder {
    std::vector<Event> events;
    void on_add(const itch::AddOrder& m) { events.emplace_back(m); }
    void on_execute(const itch::OrderExecuted& m) { events.emplace_back(m); }
    void on_cancel(const itch::OrderCancel& m) { events.emplace_back(m); }
    void on_delete(const itch::OrderDelete& m) { events.emplace_back(m); }
    void on_replace(const itch::OrderReplace& m) { events.emplace_back(m); }
};

struct MissingOne {
    void on_add(const itch::AddOrder&) {}
};

std::span<const std::byte> view(const std::vector<std::byte>& v) { return {v.data(), v.size()}; }

std::string stock_str(const std::array<char, 8>& s) { return std::string(s.data(), 8); }

}  // namespace

TEST(load_and_store_big_endian) {
    const std::byte raw[8] = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04},
                              std::byte{0x05}, std::byte{0x06}, std::byte{0x07}, std::byte{0x08}};
    CHECK_EQ(itch::load_be<std::uint16_t>(raw), 0x0102u);
    CHECK_EQ(itch::load_be<std::uint32_t>(raw), 0x01020304u);
    CHECK_EQ(itch::load_be<std::uint64_t>(raw), 0x0102030405060708ull);
    // Unaligned addresses must work too.
    CHECK_EQ(itch::load_be<std::uint32_t>(raw + 1), 0x02030405u);
    std::byte out[8] = {};
    itch::store_be<std::uint32_t>(out + 3, 0xA1B2C3D4u);
    CHECK(out[3] == std::byte{0xA1});
    CHECK(out[6] == std::byte{0xD4});
}

TEST(handler_concept) {
    CHECK_TRAIT(itch::Handler<Recorder>);
    CHECK_TRAIT(!itch::Handler<MissingOne>);
    CHECK_TRAIT(!itch::Handler<int>);
}

TEST(parse_add_order) {
    wire::Writer w;
    w.add_order(123456789, 42, 'B', 100, "AAPL", 1500250);
    Recorder r;
    auto res = itch::parse(view(w.bytes), r);
    CHECK(res.error == itch::ParseError::None);
    CHECK_EQ(res.consumed, w.bytes.size());
    CHECK_EQ(res.messages, 1u);
    CHECK_EQ(r.events.size(), 1u);
    const auto& a = std::get<itch::AddOrder>(r.events[0]);
    CHECK_EQ(a.timestamp, 123456789u);
    CHECK_EQ(a.order_ref, 42u);
    CHECK(a.side == itch::Side::Buy);
    CHECK_EQ(a.shares, 100u);
    CHECK_EQ(stock_str(a.stock), std::string("AAPL    "));
    CHECK_EQ(a.price, 1500250u);
}

TEST(parse_every_message_type) {
    wire::Writer w;
    w.add_order(1, 10, 'S', 300, "MSFT", 4000000);
    w.executed(2, 10, 100, 777);
    w.cancel(3, 10, 50);
    w.replace(4, 10, 11, 120, 4000100);
    w.del(5, 11);
    Recorder r;
    auto res = itch::parse(view(w.bytes), r);
    CHECK(res.error == itch::ParseError::None);
    CHECK_EQ(res.messages, 5u);
    CHECK_EQ(r.events.size(), 5u);
    CHECK(std::get<itch::AddOrder>(r.events[0]).side == itch::Side::Sell);
    const auto& e = std::get<itch::OrderExecuted>(r.events[1]);
    CHECK_EQ(e.order_ref, 10u);
    CHECK_EQ(e.executed_shares, 100u);
    CHECK_EQ(e.match_number, 777u);
    const auto& c = std::get<itch::OrderCancel>(r.events[2]);
    CHECK_EQ(c.cancelled_shares, 50u);
    const auto& u = std::get<itch::OrderReplace>(r.events[3]);
    CHECK_EQ(u.orig_order_ref, 10u);
    CHECK_EQ(u.new_order_ref, 11u);
    CHECK_EQ(u.shares, 120u);
    CHECK_EQ(u.price, 4000100u);
    const auto& d = std::get<itch::OrderDelete>(r.events[4]);
    CHECK_EQ(d.timestamp, 5u);
    CHECK_EQ(d.order_ref, 11u);
}

TEST(unknown_types_are_skipped) {
    wire::Writer w;
    w.unknown('S', 11);  // e.g. system event
    w.add_order(1, 1, 'B', 1, "X", 1);
    w.unknown('R', 38);  // e.g. stock directory
    w.del(2, 1);
    Recorder r;
    auto res = itch::parse(view(w.bytes), r);
    CHECK(res.error == itch::ParseError::None);
    CHECK_EQ(res.consumed, w.bytes.size());
    CHECK_EQ(res.messages, 4u);
    CHECK_EQ(res.unknown, 2u);
    CHECK_EQ(r.events.size(), 2u);
}

TEST(stops_at_incomplete_message) {
    wire::Writer w;
    w.add_order(1, 1, 'B', 1, "X", 1);
    w.add_order(2, 2, 'S', 2, "Y", 2);
    const std::size_t first = 2 + 34;
    // Every possible cut inside the second message (including inside its
    // length field) parses exactly the first message and reports how far
    // it got, so the caller can keep the rest and append more bytes later.
    for (std::size_t cut = first; cut < w.bytes.size(); ++cut) {
        Recorder r;
        auto res = itch::parse(std::span<const std::byte>(w.bytes.data(), cut), r);
        CHECK(res.error == itch::ParseError::None);
        CHECK_EQ(res.consumed, first);
        CHECK_EQ(r.events.size(), 1u);
    }
    Recorder r;
    auto res = itch::parse(std::span<const std::byte>(w.bytes.data(), 1), r);
    CHECK_EQ(res.consumed, 0u);
}

TEST(streaming_in_random_chunks) {
    // Simulate a TCP stream arriving in arbitrary pieces.
    wire::Writer w;
    std::mt19937 rng(9);
    for (int i = 0; i < 2000; ++i) {
        switch (rng() % 5) {
            case 0: w.add_order(i, i, rng() % 2 ? 'B' : 'S', rng() % 1000, "SYM", rng()); break;
            case 1: w.executed(i, i, rng() % 100, i); break;
            case 2: w.cancel(i, i, rng() % 100); break;
            case 3: w.del(i, i); break;
            case 4: w.replace(i, i, i + 1, rng() % 100, rng()); break;
        }
    }
    Recorder whole;
    itch::parse(view(w.bytes), whole);
    CHECK_EQ(whole.events.size(), 2000u);

    Recorder chunked;
    std::vector<std::byte> pending;
    std::size_t pos = 0;
    while (pos < w.bytes.size()) {
        std::size_t n = std::min<std::size_t>(1 + rng() % 70, w.bytes.size() - pos);
        pending.insert(pending.end(), w.bytes.begin() + pos, w.bytes.begin() + pos + n);
        pos += n;
        auto res = itch::parse(view(pending), chunked);
        CHECK(res.error == itch::ParseError::None);
        pending.erase(pending.begin(), pending.begin() + res.consumed);
    }
    CHECK(pending.empty());
    CHECK_EQ(chunked.events.size(), whole.events.size());
    for (std::size_t i = 0; i < whole.events.size(); ++i)
        CHECK_EQ(chunked.events[i].index(), whole.events[i].index());
}

TEST(rejects_wrong_length_for_known_type) {
    wire::Writer w;
    w.del(1, 1);
    // An 'A' message that claims 20 bytes: malformed.
    w.u16(20);
    w.ch('A');
    for (int i = 0; i < 19; ++i) w.u8(0);
    w.del(2, 2);
    Recorder r;
    auto res = itch::parse(view(w.bytes), r);
    CHECK(res.error == itch::ParseError::BadLength);
    CHECK_EQ(res.consumed, 2u + 17u);  // stopped before the bad message
    CHECK_EQ(r.events.size(), 1u);
}

TEST(rejects_zero_length) {
    wire::Writer w;
    w.u16(0);
    Recorder r;
    auto res = itch::parse(view(w.bytes), r);
    CHECK(res.error == itch::ParseError::BadLength);
    CHECK_EQ(res.consumed, 0u);
}

TEST(rejects_bad_side) {
    wire::Writer w;
    w.add_order(1, 1, 'Q', 1, "X", 1);
    Recorder r;
    auto res = itch::parse(view(w.bytes), r);
    CHECK(res.error == itch::ParseError::BadSide);
    CHECK_EQ(r.events.size(), 0u);
}

TEST(parses_from_unaligned_buffer) {
    wire::Writer w;
    w.add_order(0x1122334455667788ull, 0x99, 'B', 7, "ALIGN", 9);
    // Copy to an odd address. A reinterpret_cast-to-struct parser trips UBSan here.
    std::vector<std::byte> buf(w.bytes.size() + 1);
    std::memcpy(buf.data() + 1, w.bytes.data(), w.bytes.size());
    Recorder r;
    itch::parse(std::span<const std::byte>(buf.data() + 1, w.bytes.size()), r);
    CHECK_EQ(r.events.size(), 1u);
    CHECK_EQ(std::get<itch::AddOrder>(r.events[0]).timestamp, 0x1122334455667788ull);
}

TEST(encode_matches_wire_format) {
    std::byte buf[64];
    itch::AddOrder a{111, 222, itch::Side::Sell, 333, {'T', 'S', 'L', 'A', ' ', ' ', ' ', ' '}, 444};
    std::size_t n = itch::encode(a, buf);
    wire::Writer w;
    w.add_order(111, 222, 'S', 333, "TSLA", 444);
    CHECK_EQ(n, w.bytes.size());
    CHECK(std::memcmp(buf, w.bytes.data(), n) == 0);

    wire::Writer all;
    all.executed(1, 2, 3, 4);
    all.cancel(5, 6, 7);
    all.del(8, 9);
    all.replace(10, 11, 12, 13, 14);
    std::vector<std::byte> mine(all.bytes.size());
    std::size_t off = 0;
    off += itch::encode(itch::OrderExecuted{1, 2, 3, 4}, std::span(mine).subspan(off));
    off += itch::encode(itch::OrderCancel{5, 6, 7}, std::span(mine).subspan(off));
    off += itch::encode(itch::OrderDelete{8, 9}, std::span(mine).subspan(off));
    off += itch::encode(itch::OrderReplace{10, 11, 12, 13, 14}, std::span(mine).subspan(off));
    CHECK_EQ(off, all.bytes.size());
    CHECK(mine == all.bytes);
}

TEST(encode_refuses_small_buffer) {
    std::byte buf[10];
    CHECK_EQ(itch::encode(itch::OrderDelete{1, 2}, buf), 0u);
}

TEST(round_trip_random) {
    std::mt19937_64 rng(5);
    std::vector<std::byte> buf(64 * 1000);
    std::vector<itch::AddOrder> sent;
    std::size_t off = 0;
    for (int i = 0; i < 1000; ++i) {
        itch::AddOrder a{rng(), rng(), rng() % 2 ? itch::Side::Buy : itch::Side::Sell,
                         static_cast<std::uint32_t>(rng()), {}, static_cast<std::uint32_t>(rng())};
        for (auto& c : a.stock) c = char('A' + rng() % 26);
        sent.push_back(a);
        off += itch::encode(a, std::span(buf).subspan(off));
    }
    Recorder r;
    auto res = itch::parse(std::span<const std::byte>(buf.data(), off), r);
    CHECK_EQ(res.consumed, off);
    CHECK_EQ(r.events.size(), sent.size());
    for (std::size_t i = 0; i < sent.size(); ++i) {
        const auto& got = std::get<itch::AddOrder>(r.events[i]);
        CHECK_EQ(got.order_ref, sent[i].order_ref);
        CHECK_EQ(got.price, sent[i].price);
        CHECK(got.stock == sent[i].stock);
        CHECK(got.side == sent[i].side);
    }
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
