#pragma once

#include <pthread.h>
#include <sched.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include "../12_spsc_queue/spsc_queue.hpp"
#include "../13_seqlock/seqlock.hpp"
#include "../15_itch_parser/itch_parser.hpp"
#include "../17_l2_order_book/order_book.hpp"

namespace hft::pipeline {

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
    std::string stock;
    int book_cpu = -1;  // pin the book thread to this CPU if >= 0
};

inline void spin_pause() noexcept {
#if defined(__x86_64__) || defined(__i386__)
    __builtin_ia32_pause();
#endif
}

inline bool pin_current_thread(int cpu) {
    if (cpu < 0 || cpu >= CPU_SETSIZE) return false;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    return pthread_setaffinity_np(pthread_self(), sizeof set, &set) == 0;
}

class Pipeline {
    using Event = std::variant<itch::AddOrder, itch::OrderExecuted, itch::OrderCancel,
                               itch::OrderDelete, itch::OrderReplace>;
    static constexpr std::size_t kQueueSize = 4096;

    // Feed side: turns parsed messages into queue pushes.
    struct Pusher {
        Pipeline* p;
        template <typename M>
        void push(const M& m) {
            while (!p->queue_->try_push(Event{m})) spin_pause();
        }
        void on_add(const itch::AddOrder& m) { push(m); }
        void on_execute(const itch::OrderExecuted& m) { push(m); }
        void on_cancel(const itch::OrderCancel& m) { push(m); }
        void on_delete(const itch::OrderDelete& m) { push(m); }
        void on_replace(const itch::OrderReplace& m) { push(m); }
    };

public:
    explicit Pipeline(Config cfg)
        : cfg_(std::move(cfg)),
          queue_(std::make_unique<SpscQueue<Event, kQueueSize>>()),
          builder_(book_, cfg_.stock) {
        pending_.reserve(1 << 16);
    }

    ~Pipeline() { stop(); }

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void start() {
        if (thread_.joinable()) return;
        stop_.store(false, std::memory_order_relaxed);
        thread_ = std::thread([this] { run(); });
    }

    void on_bytes(std::span<const std::byte> chunk) {
        Pusher pusher{this};
        if (pending_.empty()) {
            auto r = itch::parse(chunk, pusher);
            pending_.insert(pending_.end(), chunk.begin() + static_cast<std::ptrdiff_t>(r.consumed),
                            chunk.end());
            return;
        }
        pending_.insert(pending_.end(), chunk.begin(), chunk.end());
        auto r = itch::parse(std::span<const std::byte>(pending_), pusher);
        pending_.erase(pending_.begin(), pending_.begin() + static_cast<std::ptrdiff_t>(r.consumed));
    }

    void stop() {
        if (!thread_.joinable()) return;
        stop_.store(true, std::memory_order_release);
        thread_.join();
    }

    TopOfBook top() const { return top_.load(); }
    const book::OrderBook& book() const { return book_; }
    std::uint64_t events_applied() const { return applied_.load(std::memory_order_acquire); }

private:
    void run() {
        if (cfg_.book_cpu >= 0) pin_current_thread(cfg_.book_cpu);
        for (;;) {
            Event* e = queue_->front();
            if (!e) {
                if (stop_.load(std::memory_order_acquire)) {
                    // Re-check: the feed may have pushed just before stop.
                    if (!queue_->front()) return;
                    continue;
                }
                spin_pause();
                continue;
            }
            std::visit([this](const auto& m) { apply(m); }, *e);
            queue_->pop();
            std::uint64_t seq = applied_.load(std::memory_order_relaxed) + 1;
            applied_.store(seq, std::memory_order_release);
            publish(seq);
        }
    }

    void apply(const itch::AddOrder& m) { builder_.on_add(m); }
    void apply(const itch::OrderExecuted& m) { builder_.on_execute(m); }
    void apply(const itch::OrderCancel& m) { builder_.on_cancel(m); }
    void apply(const itch::OrderDelete& m) { builder_.on_delete(m); }
    void apply(const itch::OrderReplace& m) { builder_.on_replace(m); }

    void publish(std::uint64_t seq) {
        TopOfBook t;
        t.sequence = seq;
        if (auto b = book_.best_bid()) {
            t.has_bid = true;
            t.bid_px = b->price;
            t.bid_qty = b->qty;
        }
        if (auto a = book_.best_ask()) {
            t.has_ask = true;
            t.ask_px = a->price;
            t.ask_qty = a->qty;
        }
        top_.store(t);
    }

    Config cfg_;
    std::unique_ptr<SpscQueue<Event, kQueueSize>> queue_;
    book::OrderBook book_;
    book::BookBuilder builder_;
    Seqlock<TopOfBook> top_;
    std::vector<std::byte> pending_;
    std::atomic<bool> stop_{false};
    std::atomic<std::uint64_t> applied_{0};
    std::thread thread_;
};

}  // namespace hft::pipeline
