// Problem 16: UDP feed handler with A/B arbitration and gap detection
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
// Linux only (epoll).
#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

namespace hft::feed {

// MoldUDP64-style packet header (20 bytes, big-endian):
//   char session[10], uint64 sequence (of the first message), uint16 count.
// Followed by `count` message blocks: uint16 length, then `length` bytes.
struct PacketHeader {
    std::array<char, 10> session;
    std::uint64_t sequence;
    std::uint16_t count;
};

inline constexpr std::size_t kHeaderSize = 20;

// nullopt if the packet is shorter than a header.
inline std::optional<PacketHeader> decode_header(std::span<const std::byte> pkt) {
    (void)pkt;
    return std::nullopt;  // TODO
}

// A handler receives in-order messages and gap notifications:
//   h.on_message(std::uint64_t seq, std::span<const std::byte> payload);
//   h.on_gap(std::uint64_t first_missing, std::uint64_t last_missing);
template <typename H>
concept FeedHandler = true;  // TODO

// Turns packets from one or more redundant lines into an exactly-once,
// in-order message stream. See README for the precise rules.
template <FeedHandler H>
class SequenceTracker {
public:
    explicit SequenceTracker(H& handler, std::uint64_t first_expected = 1,
                             std::size_t max_buffered = 64)
        : h_(handler) {
        (void)first_expected;
        (void)max_buffered;
    }

    void on_packet(std::span<const std::byte> pkt) {
        (void)pkt;
        throw std::logic_error("TODO: on_packet");
    }

    std::uint64_t next_expected() const noexcept { return 0; /* TODO */ }
    std::uint64_t duplicates() const noexcept { return 0; /* TODO */ }
    std::uint64_t gaps() const noexcept { return 0; /* TODO */ }
    std::uint64_t lost() const noexcept { return 0; /* TODO */ }
    std::uint64_t malformed() const noexcept { return 0; /* TODO */ }
    std::size_t buffered() const noexcept { return 0; /* TODO */ }

private:
    H& h_;
};

// Non-blocking UDP socket bound to bind_ip:port (port 0 = ephemeral),
// with its own epoll instance.
class UdpReceiver {
public:
    // Throws std::system_error on any failure (bad address, bind, ...).
    explicit UdpReceiver(std::uint16_t port = 0, const char* bind_ip = "127.0.0.1") {
        (void)port;
        (void)bind_ip;
        throw std::logic_error("TODO: UdpReceiver");
    }

    // TODO: non-copyable; closes its file descriptors in the destructor.

    std::uint16_t port() const noexcept { return 0; /* TODO: the actual bound port */ }
    int fd() const noexcept { return -1; /* TODO */ }

    // Waits up to timeout_ms for data, then reads datagrams until the socket
    // would block, calling on_datagram(std::span<const std::byte>) for each.
    // Returns the number of datagrams delivered. No allocation per datagram.
    template <typename F>
    std::size_t poll(int timeout_ms, F&& on_datagram) {
        (void)timeout_ms;
        (void)on_datagram;
        throw std::logic_error("TODO: poll");
    }
};

}  // namespace hft::feed
