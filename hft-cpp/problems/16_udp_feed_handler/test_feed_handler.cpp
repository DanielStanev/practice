#include "feed_handler.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <random>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "itch_wire.hpp"
#include "alloc_counter.hpp"
#include "testing.hpp"

using hft::feed::SequenceTracker;
using hft::feed::UdpReceiver;


namespace {

// Each message payload is 8 bytes: its own sequence number, so the
// recorder can check that payloads match sequence numbers.
std::vector<std::byte> payload_for(std::uint64_t seq) {
    wire::Writer w;
    w.u64(seq);
    return w.bytes;
}

std::vector<std::byte> make_packet(std::uint64_t first, std::uint16_t count) {
    wire::Packet p("SESSION001", first, count);
    for (std::uint16_t i = 0; i < count; ++i) p.add_raw(payload_for(first + i));
    return p.bytes;
}

struct Recorder {
    std::vector<std::uint64_t> seqs;
    std::vector<std::pair<std::uint64_t, std::uint64_t>> gaps;
    bool payload_mismatch = false;
    void on_message(std::uint64_t seq, std::span<const std::byte> msg) {
        if (msg.size() != 8) {
            payload_mismatch = true;
        } else {
            std::uint64_t v = 0;
            for (auto b : msg) v = (v << 8) | std::to_integer<std::uint64_t>(b);
            if (v != seq) payload_mismatch = true;
        }
        seqs.push_back(seq);
    }
    void on_gap(std::uint64_t from, std::uint64_t to) { gaps.emplace_back(from, to); }
};

std::vector<std::uint64_t> range(std::uint64_t from, std::uint64_t to_inclusive) {
    std::vector<std::uint64_t> v;
    for (auto s = from; s <= to_inclusive; ++s) v.push_back(s);
    return v;
}

std::span<const std::byte> view(const std::vector<std::byte>& v) { return {v.data(), v.size()}; }

}  // namespace

// ------------------------------------------------------------ header

TEST(decode_header) {
    auto pkt = make_packet(0x0102030405060708ull, 3);
    auto h = hft::feed::decode_header(view(pkt));
    CHECK(h.has_value());
    CHECK_EQ(std::string(h->session.data(), 10), std::string("SESSION001"));
    CHECK_EQ(h->sequence, 0x0102030405060708ull);
    CHECK_EQ(h->count, 3u);
    CHECK_FALSE(hft::feed::decode_header(std::span(pkt).first(19)).has_value());
}

// ------------------------------------------------------------ sequencing

TEST(in_order_delivery) {
    Recorder r;
    SequenceTracker t(r);
    CHECK_EQ(t.next_expected(), 1u);
    t.on_packet(view(make_packet(1, 3)));
    t.on_packet(view(make_packet(4, 2)));
    CHECK(r.seqs == range(1, 5));
    CHECK_FALSE(r.payload_mismatch);
    CHECK_EQ(t.next_expected(), 6u);
    CHECK(r.gaps.empty());
    CHECK_EQ(t.duplicates(), 0u);
}

TEST(custom_first_sequence) {
    Recorder r;
    SequenceTracker t(r, 1000);
    t.on_packet(view(make_packet(1000, 2)));
    CHECK(r.seqs == range(1000, 1001));
}

TEST(duplicate_packets_dropped) {
    Recorder r;
    SequenceTracker t(r);
    auto p = make_packet(1, 4);
    t.on_packet(view(p));
    t.on_packet(view(p));
    CHECK(r.seqs == range(1, 4));
    CHECK_EQ(t.duplicates(), 4u);
}

TEST(overlapping_packet_delivers_only_new) {
    Recorder r;
    SequenceTracker t(r);
    t.on_packet(view(make_packet(1, 5)));
    t.on_packet(view(make_packet(4, 4)));  // 4..7, of which 4 and 5 are old
    CHECK(r.seqs == range(1, 7));
    CHECK_EQ(t.duplicates(), 2u);
    CHECK_FALSE(r.payload_mismatch);
}

TEST(gap_is_reported_buffered_and_filled) {
    Recorder r;
    SequenceTracker t(r);
    t.on_packet(view(make_packet(4, 2)));  // 1..3 missing
    CHECK(r.seqs.empty());
    CHECK_EQ(r.gaps.size(), 1u);
    CHECK_EQ(r.gaps[0].first, 1u);
    CHECK_EQ(r.gaps[0].second, 3u);
    CHECK_EQ(t.buffered(), 1u);
    CHECK_EQ(t.next_expected(), 1u);
    t.on_packet(view(make_packet(1, 3)));  // fill
    CHECK(r.seqs == range(1, 5));
    CHECK_EQ(t.buffered(), 0u);
    CHECK_EQ(t.next_expected(), 6u);
    CHECK_EQ(t.gaps(), 1u);
}

TEST(gap_reported_once) {
    Recorder r;
    SequenceTracker t(r);
    t.on_packet(view(make_packet(5, 1)));
    t.on_packet(view(make_packet(6, 1)));
    t.on_packet(view(make_packet(7, 1)));
    CHECK_EQ(r.gaps.size(), 1u);  // 1..4, not re-reported for each packet
    t.on_packet(view(make_packet(10, 1)));  // a second, new gap: 8..9
    CHECK_EQ(r.gaps.size(), 2u);
    CHECK_EQ(r.gaps[1].first, 8u);
    CHECK_EQ(r.gaps[1].second, 9u);
    t.on_packet(view(make_packet(1, 4)));
    CHECK(r.seqs == range(1, 7));
    t.on_packet(view(make_packet(8, 2)));
    CHECK(r.seqs == range(1, 10));
    CHECK_EQ(t.buffered(), 0u);
}

TEST(out_of_order_buffered_packets_drain_in_sequence) {
    Recorder r;
    SequenceTracker t(r);
    t.on_packet(view(make_packet(7, 2)));
    t.on_packet(view(make_packet(3, 4)));
    t.on_packet(view(make_packet(9, 1)));
    t.on_packet(view(make_packet(1, 2)));
    CHECK(r.seqs == range(1, 9));
    CHECK_FALSE(r.payload_mismatch);
}

TEST(heartbeat_detects_gap) {
    Recorder r;
    SequenceTracker t(r);
    t.on_packet(view(make_packet(1, 2)));
    t.on_packet(view(make_packet(3, 0)));  // heartbeat, next seq is 3: fine
    CHECK(r.gaps.empty());
    t.on_packet(view(make_packet(6, 0)));  // heartbeat says 3..5 were sent
    CHECK_EQ(r.gaps.size(), 1u);
    CHECK_EQ(r.gaps[0].first, 3u);
    CHECK_EQ(r.gaps[0].second, 5u);
    CHECK_EQ(t.buffered(), 0u);
}

TEST(gives_up_when_buffer_full) {
    Recorder r;
    SequenceTracker t(r, 1, 4);
    for (std::uint64_t s = 3; s <= 7; ++s) t.on_packet(view(make_packet(s, 1)));
    // 1..2 never arrived; after the 5th buffered packet the tracker stops
    // waiting, counts them lost, and moves on.
    CHECK_EQ(t.lost(), 2u);
    CHECK(r.seqs == range(3, 7));
    CHECK_EQ(t.next_expected(), 8u);
    CHECK_EQ(t.buffered(), 0u);
    t.on_packet(view(make_packet(1, 2)));  // late arrival: duplicate now
    CHECK_EQ(t.duplicates(), 2u);
    CHECK(r.seqs == range(3, 7));
}

TEST(malformed_packets_ignored) {
    Recorder r;
    SequenceTracker t(r);
    auto short_pkt = make_packet(1, 1);
    short_pkt.resize(10);
    t.on_packet(view(short_pkt));
    auto lying = make_packet(1, 2);
    lying[19] = std::byte{3};  // header claims 3 messages, only 2 present
    t.on_packet(view(lying));
    auto truncated = make_packet(1, 2);
    truncated.pop_back();
    t.on_packet(view(truncated));
    CHECK_EQ(t.malformed(), 3u);
    CHECK(r.seqs.empty());
    CHECK_EQ(t.next_expected(), 1u);
    t.on_packet(view(make_packet(1, 2)));
    CHECK(r.seqs == range(1, 2));
}

TEST(ab_line_arbitration) {
    // The exchange sends identical packets on two lines (A and B). Each line
    // independently drops ~20% of packets and they arrive interleaved and
    // slightly reordered. Every message must come out exactly once, in order,
    // as long as at least one line delivered it.
    std::mt19937 rng(21);
    std::vector<std::vector<std::byte>> packets;
    std::uint64_t seq = 1;
    for (int i = 0; i < 3000; ++i) {
        std::uint16_t n = std::uint16_t(1 + rng() % 4);
        packets.push_back(make_packet(seq, n));
        seq += n;
    }
    std::vector<std::pair<int, const std::vector<std::byte>*>> arrivals;  // (time, packet)
    for (std::size_t i = 0; i < packets.size(); ++i) {
        bool a = rng() % 5 != 0, b = rng() % 5 != 0;
        if (!a && !b) a = true;  // at least one line delivers
        if (a) arrivals.emplace_back(int(i * 10 + rng() % 15), &packets[i]);
        if (b) arrivals.emplace_back(int(i * 10 + rng() % 15), &packets[i]);
    }
    std::stable_sort(arrivals.begin(), arrivals.end(),
                     [](auto& x, auto& y) { return x.first < y.first; });
    Recorder r;
    SequenceTracker t(r, 1, 1024);
    for (auto& [time, pkt] : arrivals) t.on_packet(view(*pkt));
    CHECK(r.seqs == range(1, seq - 1));
    CHECK_FALSE(r.payload_mismatch);
    CHECK_EQ(t.lost(), 0u);
    CHECK_EQ(t.buffered(), 0u);
}

TEST(in_order_path_does_not_allocate) {
    std::vector<std::vector<std::byte>> packets;
    for (std::uint64_t s = 1; s <= 10000; s += 3) packets.push_back(make_packet(s, 3));
    struct Counter {
        std::uint64_t n = 0;
        void on_message(std::uint64_t, std::span<const std::byte>) { ++n; }
        void on_gap(std::uint64_t, std::uint64_t) {}
    } c;
    SequenceTracker t(c);
    long before = alloc_counter::count();
    for (auto& p : packets) t.on_packet(view(p));
    CHECK_EQ(alloc_counter::count(), before);
    CHECK_EQ(c.n, packets.size() * 3);
}

// ------------------------------------------------------------ UDP socket

namespace {
struct Sender {
    int fd;
    sockaddr_in to{};
    explicit Sender(std::uint16_t port) {
        fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        to.sin_family = AF_INET;
        to.sin_port = htons(port);
        ::inet_pton(AF_INET, "127.0.0.1", &to.sin_addr);
    }
    ~Sender() { ::close(fd); }
    void send(const std::vector<std::byte>& b) {
        ::sendto(fd, b.data(), b.size(), 0, reinterpret_cast<sockaddr*>(&to), sizeof to);
    }
};
}  // namespace

TEST(udp_binds_ephemeral_port) {
    UdpReceiver rx;  // port 0 = let the kernel choose
    CHECK(rx.port() != 0);
    CHECK(rx.fd() >= 0);
}

TEST(udp_poll_times_out_without_data) {
    UdpReceiver rx;
    auto t0 = std::chrono::steady_clock::now();
    std::size_t n = rx.poll(20, [](std::span<const std::byte>) {});
    auto elapsed = std::chrono::steady_clock::now() - t0;
    CHECK_EQ(n, 0u);
    CHECK(elapsed >= std::chrono::milliseconds(15));
}

TEST(udp_receives_datagrams_intact) {
    UdpReceiver rx;
    Sender tx(rx.port());
    std::vector<std::vector<std::byte>> sent;
    for (int i = 0; i < 50; ++i) {
        std::vector<std::byte> d(std::size_t(1 + i * 27));
        for (std::size_t k = 0; k < d.size(); ++k) d[k] = std::byte(k * 7 + i);
        sent.push_back(d);
        tx.send(d);
    }
    std::vector<std::vector<std::byte>> got;
    for (int tries = 0; tries < 100 && got.size() < sent.size(); ++tries)
        rx.poll(10, [&](std::span<const std::byte> d) { got.emplace_back(d.begin(), d.end()); });
    CHECK_EQ(got.size(), sent.size());
    CHECK(got == sent);  // loopback preserves order
}

TEST(udp_poll_drains_everything_available) {
    UdpReceiver rx;
    Sender tx(rx.port());
    for (int i = 0; i < 20; ++i) tx.send(make_packet(std::uint64_t(i), 0));
    // Give the kernel a moment, then one poll should return all of them
    // (edge of the requirement: drain until EAGAIN, not one per poll).
    std::size_t n = 0;
    for (int tries = 0; tries < 50 && n == 0; ++tries) n = rx.poll(10, [](auto) {});
    CHECK_EQ(n, 20u);
}

TEST(udp_end_to_end_with_tracker) {
    UdpReceiver rx;
    Sender tx(rx.port());
    Recorder r;
    SequenceTracker t(r);
    std::uint64_t seq = 1;
    for (int i = 0; i < 200; ++i) {
        tx.send(make_packet(seq, 2));
        seq += 2;
    }
    for (int tries = 0; tries < 200 && r.seqs.size() < 400; ++tries)
        rx.poll(10, [&](std::span<const std::byte> d) { t.on_packet(d); });
    CHECK(r.seqs == range(1, 400));
}

TEST(udp_invalid_address_throws) {
    // Binding to an invalid address must throw std::system_error.
    CHECK_THROWS_AS(UdpReceiver(0, "not-an-ip"), std::system_error);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
