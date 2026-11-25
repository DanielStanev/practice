# 16 · UDP feed handler: sockets, epoll, A/B arbitration, gap detection

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★★☆ | 15 (binary framing), 07 (no allocation on the hot path) | non-blocking UDP sockets, `epoll`, MoldUDP64 sequencing, A/B line arbitration, gap detection and recovery policy, Linux system call error handling |

## Why this matters

Exchanges multicast market data over UDP, which can drop, duplicate and
reorder packets. To survive that they publish the same feed on two lines
(A and B) and number every message. A feed handler merges both lines into
one exactly-once, in-order stream, notices gaps, and decides how long to
wait for a missing packet before giving up and resynchronising. This is the
first piece of real HFT infrastructure in the course, and "how do you handle
packet loss on a multicast feed?" is a standard systems interview question.

## Task

Implement `decode_header`, the `FeedHandler` concept, `SequenceTracker<H>`
and `UdpReceiver` in `feed_handler.hpp`.

## Packet format (MoldUDP64-style, big-endian)

| Off | Size | Field |
|---|---|---|
| 0 | 10 | session (ASCII) |
| 10 | 8 | sequence number of the first message in this packet |
| 18 | 2 | message count (0 = heartbeat) |
| 20 | … | `count` × { uint16 length, `length` bytes of message } |

A packet with sequence `s` and count `n` carries messages `s … s+n-1`.
A heartbeat with sequence `s` says "the next message will be `s`".

## SequenceTracker rules

`next_expected()` starts at `first_expected`. For each packet:

1. **Malformed** (shorter than a header, or message blocks that overrun the
   packet): count in `malformed()` and ignore.
2. **Entirely old** (`s + n <= next_expected`): count its messages as
   `duplicates()` and ignore. This is the normal case for the slower of
   the two lines.
3. **Contains the next expected message** (`s <= next_expected < s + n`):
   deliver messages from `next_expected` on via `on_message(seq, payload)`,
   counting the earlier ones as duplicates. Then deliver any buffered
   packets that have become contiguous.
4. **From the future** (`s > next_expected`): a gap. Call
   `on_gap(first_missing, s - 1)` once for each newly discovered missing
   range (do not re-report a range on every later packet; do not report
   messages you have already received or buffered). Then copy the packet
   into a buffer (heartbeats are not buffered). A packet already buffered
   counts as duplicates.
5. **Buffer limit**: if more than `max_buffered` packets are waiting, give
   up on the missing messages: add them to `lost()`, jump `next_expected` to
   the oldest buffered packet, and deliver from there.
6. **Hot path**: in-order packets must not allocate (tests count
   `operator new`). Only the gap path may copy.

## UdpReceiver

- Non-blocking UDP socket bound to `bind_ip:port` (port 0 = ephemeral;
  `port()` reports the real one) plus an `epoll` instance.
- `poll(timeout_ms, f)` waits up to `timeout_ms`, then reads datagrams until
  `EAGAIN`, calling `f(std::span<const std::byte>)` for each, from a
  reusable buffer. Returns the count.
- Every failing system call throws `std::system_error` with `errno`.
  File descriptors are closed in the destructor (RAII, problem 01).

## Running

```bash
./run_tests.sh
```

The UDP tests use the loopback interface.

<details>
<summary>Hints</summary>

- A `std::map<uint64_t, std::vector<std::byte>>` keyed by first sequence is
  a perfectly good gap buffer: it is off the hot path.
- Keep a "highest sequence seen" watermark. A newly discovered gap starts at
  `max(next_expected, watermark)`.
- `socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)`, `bind`,
  `getsockname` for the port, `epoll_create1`, `epoll_ctl(EPOLL_CTL_ADD)`.
  Raise `SO_RCVBUF`: a full kernel buffer is where real packet loss happens.
</details>

## Going further

- Join a multicast group (`IP_ADD_MEMBERSHIP`) and run two receivers on two
  groups as real A and B lines.
- Read many datagrams per syscall with `recvmmsg` and measure the difference.
- Get kernel receive timestamps with `SO_TIMESTAMPNS` and compute
  wire-to-handler latency with your histogram from problem 09.
- Read about kernel-bypass networking: Solarflare/AMD OpenOnload and
  ef_vi, DPDK, and why HFT firms use them.

## Reading

- Nasdaq MoldUDP64 protocol specification (PDF on nasdaqtrader.com)
- W. Richard Stevens, *UNIX Network Programming, Vol. 1*, chapters 8 (UDP) and 21 (multicast)
- `man 7 epoll`, `man 7 udp`, `man 2 recvmmsg`
- Cloudflare blog, "How to receive a million packets per second"
