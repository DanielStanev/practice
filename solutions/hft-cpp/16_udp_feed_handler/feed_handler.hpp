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
#include <system_error>
#include <utility>
#include <vector>

namespace hft::feed {

struct PacketHeader {
    std::array<char, 10> session;
    std::uint64_t sequence;
    std::uint16_t count;
};

inline constexpr std::size_t kHeaderSize = 20;

namespace detail {
inline std::uint16_t be16(const std::byte* p) {
    return static_cast<std::uint16_t>((std::to_integer<unsigned>(p[0]) << 8) |
                                      std::to_integer<unsigned>(p[1]));
}
inline std::uint64_t be64(const std::byte* p) {
    std::uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | std::to_integer<std::uint64_t>(p[i]);
    return v;
}
}  // namespace detail

inline std::optional<PacketHeader> decode_header(std::span<const std::byte> pkt) {
    if (pkt.size() < kHeaderSize) return std::nullopt;
    PacketHeader h;
    std::memcpy(h.session.data(), pkt.data(), 10);
    h.sequence = detail::be64(pkt.data() + 10);
    h.count = detail::be16(pkt.data() + 18);
    return h;
}

template <typename H>
concept FeedHandler = requires(H& h, std::uint64_t seq, std::span<const std::byte> msg) {
    h.on_message(seq, msg);
    h.on_gap(seq, seq);
};

template <FeedHandler H>
class SequenceTracker {
public:
    explicit SequenceTracker(H& handler, std::uint64_t first_expected = 1, std::size_t max_buffered = 64)
        : h_(handler), expected_(first_expected), max_buffered_(max_buffered), highest_end_(first_expected) {}

    void on_packet(std::span<const std::byte> pkt) {
        auto hdr = decode_header(pkt);
        if (!hdr || !well_formed(pkt, hdr->count)) {
            ++malformed_;
            return;
        }
        const std::uint64_t first = hdr->sequence;
        const std::uint64_t end = first + hdr->count;

        if (first > expected_) {
            report_gap(first);
            highest_end_ = std::max(highest_end_, end);
            if (hdr->count == 0) return;  // heartbeat: nothing to buffer
            if (!buffer_.emplace(first, std::vector<std::byte>(pkt.begin(), pkt.end())).second) {
                duplicates_ += hdr->count;  // same packet already buffered (other line)
                return;
            }
            if (buffer_.size() > max_buffered_) give_up();
            return;
        }
        highest_end_ = std::max(highest_end_, end);
        if (end <= expected_) {
            duplicates_ += hdr->count;
            return;
        }
        deliver(pkt, first, hdr->count);
        drain();
    }

    std::uint64_t next_expected() const noexcept { return expected_; }
    std::uint64_t duplicates() const noexcept { return duplicates_; }
    std::uint64_t gaps() const noexcept { return gaps_; }
    std::uint64_t lost() const noexcept { return lost_; }
    std::uint64_t malformed() const noexcept { return malformed_; }
    std::size_t buffered() const noexcept { return buffer_.size(); }

private:
    static bool well_formed(std::span<const std::byte> pkt, std::uint16_t count) {
        std::size_t off = kHeaderSize;
        for (std::uint16_t i = 0; i < count; ++i) {
            if (off + 2 > pkt.size()) return false;
            std::size_t len = detail::be16(pkt.data() + off);
            off += 2 + len;
            if (off > pkt.size()) return false;
        }
        return true;
    }

    // Reports [max(expected, everything seen so far), first) once.
    void report_gap(std::uint64_t first) {
        std::uint64_t from = std::max(expected_, highest_end_);
        if (from >= first) return;
        ++gaps_;
        h_.on_gap(from, first - 1);
    }

    // Delivers messages of a packet starting at `first` from expected_ on.
    void deliver(std::span<const std::byte> pkt, std::uint64_t first, std::uint16_t count) {
        std::size_t off = kHeaderSize;
        for (std::uint16_t i = 0; i < count; ++i) {
            std::size_t len = detail::be16(pkt.data() + off);
            std::uint64_t seq = first + i;
            if (seq < expected_) {
                ++duplicates_;
            } else {
                h_.on_message(seq, pkt.subspan(off + 2, len));
                expected_ = seq + 1;
            }
            off += 2 + len;
        }
    }

    void drain() {
        while (!buffer_.empty()) {
            auto it = buffer_.begin();
            if (it->first > expected_) return;
            std::vector<std::byte> bytes = std::move(it->second);
            buffer_.erase(it);
            auto hdr = decode_header(bytes);
            if (hdr->sequence + hdr->count <= expected_) {
                duplicates_ += hdr->count;
                continue;
            }
            deliver(bytes, hdr->sequence, hdr->count);
        }
    }

    void give_up() {
        // Too much buffered: declare the missing messages lost and jump to the
        // oldest buffered packet.
        std::uint64_t jump = buffer_.begin()->first;
        lost_ += jump - expected_;
        expected_ = jump;
        drain();
    }

    H& h_;
    std::uint64_t expected_;
    std::size_t max_buffered_;
    std::uint64_t highest_end_;
    std::uint64_t duplicates_ = 0;
    std::uint64_t gaps_ = 0;
    std::uint64_t lost_ = 0;
    std::uint64_t malformed_ = 0;
    std::map<std::uint64_t, std::vector<std::byte>> buffer_;
};

class UdpReceiver {
public:
    explicit UdpReceiver(std::uint16_t port = 0, const char* bind_ip = "127.0.0.1") {
        fd_ = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        if (fd_ < 0) throw std::system_error(errno, std::generic_category(), "socket");
        int one = 1;
        ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
        int rcvbuf = 4 << 20;
        ::setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof rcvbuf);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if (::inet_pton(AF_INET, bind_ip, &addr.sin_addr) != 1) {
            ::close(fd_);
            throw std::system_error(EINVAL, std::generic_category(), "inet_pton");
        }
        if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0) {
            int e = errno;
            ::close(fd_);
            throw std::system_error(e, std::generic_category(), "bind");
        }
        socklen_t len = sizeof addr;
        ::getsockname(fd_, reinterpret_cast<sockaddr*>(&addr), &len);
        port_ = ntohs(addr.sin_port);
        epfd_ = ::epoll_create1(EPOLL_CLOEXEC);
        if (epfd_ < 0) {
            int e = errno;
            ::close(fd_);
            throw std::system_error(e, std::generic_category(), "epoll_create1");
        }
        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = fd_;
        ::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd_, &ev);
    }

    UdpReceiver(const UdpReceiver&) = delete;
    UdpReceiver& operator=(const UdpReceiver&) = delete;

    UdpReceiver(UdpReceiver&& o) noexcept
        : fd_(std::exchange(o.fd_, -1)), epfd_(std::exchange(o.epfd_, -1)), port_(o.port_) {}
    UdpReceiver& operator=(UdpReceiver&& o) noexcept {
        if (this != &o) {
            close_all();
            fd_ = std::exchange(o.fd_, -1);
            epfd_ = std::exchange(o.epfd_, -1);
            port_ = o.port_;
        }
        return *this;
    }

    ~UdpReceiver() { close_all(); }

    std::uint16_t port() const noexcept { return port_; }
    int fd() const noexcept { return fd_; }

    template <typename F>
    std::size_t poll(int timeout_ms, F&& on_datagram) {
        epoll_event ev;
        int n = ::epoll_wait(epfd_, &ev, 1, timeout_ms);
        if (n <= 0) return 0;
        std::size_t got = 0;
        for (;;) {
            ssize_t r = ::recv(fd_, buf_.data(), buf_.size(), 0);
            if (r < 0) {
                if (errno == EINTR) continue;
                break;  // EAGAIN: drained
            }
            on_datagram(std::span<const std::byte>(buf_.data(), static_cast<std::size_t>(r)));
            ++got;
        }
        return got;
    }

private:
    void close_all() noexcept {
        if (epfd_ >= 0) ::close(epfd_);
        if (fd_ >= 0) ::close(fd_);
        epfd_ = fd_ = -1;
    }

    int fd_ = -1;
    int epfd_ = -1;
    std::uint16_t port_ = 0;
    std::array<std::byte, 65536> buf_{};
};

}  // namespace hft::feed
