// Problem 19: feed-to-book pipeline across two threads
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
//
// This problem reuses your solutions to 12, 13, 15 and 17. It will only
// pass once those do.
#pragma once

#include <pthread.h>
#include <sched.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include "../12_spsc_queue/spsc_queue.hpp"
#include "../13_seqlock/seqlock.hpp"
#include "../15_itch_parser/itch_parser.hpp"
#include "../17_l2_order_book/order_book.hpp"

namespace hft::pipeline {

// Published by the book thread after every event it applies.
struct TopOfBook {
    std::uint64_t sequence = 0;  // number of events applied so far
    bool has_bid = false;
    bool has_ask = false;
    std::int64_t bid_px = 0;
    std::uint64_t bid_qty = 0;
    std::int64_t ask_px = 0;
    std::uint64_t ask_qty = 0;
    friend bool operator==(const TopOfBook&, const TopOfBook&) = default;
};

struct Config {
    std::string stock;  // only Add Orders for this stock enter the book
    int book_cpu = -1;  // pin the book thread to this CPU if >= 0
};

// Pins the calling thread to one CPU. Returns false on failure (including
// cpu < 0 or cpu >= CPU_SETSIZE).
inline bool pin_current_thread(int cpu) {
    (void)cpu;
    return false;  // TODO: pthread_setaffinity_np
}

// Feed thread (whoever calls on_bytes) -> SPSC queue -> book thread.
//
//   on_bytes()  parses ITCH (problem 15), pushes decoded events into an
//               SpscQueue (problem 12), spinning if it is full.
//   book thread pops events, applies them to an OrderBook through a
//               BookBuilder (problem 17), and after EVERY event publishes a
//               TopOfBook through a Seqlock (problem 13).
//   top()       may be called from any thread at any time.
class Pipeline {
public:
    explicit Pipeline(Config cfg) : cfg_(std::move(cfg)) {}

    // Stops the book thread if it is running.
    ~Pipeline() {
        // TODO
    }

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    // Launches the book thread (pinned if cfg.book_cpu >= 0). The pipeline
    // can be started again after stop(); book state carries over.
    void start() { throw std::logic_error("TODO: start"); }

    // Called from the feed thread with an arbitrary chunk of the ITCH byte
    // stream. Messages may be split across chunks.
    void on_bytes(std::span<const std::byte> chunk) {
        (void)chunk;
        throw std::logic_error("TODO: on_bytes");
    }

    // Lets the book thread apply every event already pushed, then joins it.
    // Idempotent; a no-op if not running.
    void stop() {
        // TODO
    }

    // Latest published snapshot. Safe from any thread.
    TopOfBook top() const { throw std::logic_error("TODO: top"); }

    // The book itself. Only call when the book thread is stopped.
    const book::OrderBook& book() const { throw std::logic_error("TODO: book"); }

    // Number of events the book thread has applied (any thread).
    std::uint64_t events_applied() const { throw std::logic_error("TODO: events_applied"); }

private:
    Config cfg_;
    // TODO: the queue (heap-allocate it: it is big), the book and builder,
    // the seqlock, a buffer for partial messages, a stop flag, the thread.
};

}  // namespace hft::pipeline
