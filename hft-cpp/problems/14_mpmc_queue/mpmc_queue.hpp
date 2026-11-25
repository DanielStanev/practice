// Problem 14: bounded lock-free MPMC queue (Dmitry Vyukov's design)
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
//
// Any number of threads may call try_push / try_emplace / try_pop concurrently.
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

namespace hft {

template <typename T>
class MpmcQueue {
public:
    // capacity must be a power of two >= 2, otherwise std::invalid_argument.
    // All storage is allocated here.
    explicit MpmcQueue(std::size_t capacity) {
        (void)capacity;
        // TODO
    }

    // TODO: non-copyable

    // Destroys elements still in the queue and frees storage.
    ~MpmcQueue() {
        // TODO
    }

    std::size_t capacity() const noexcept { return 0; /* TODO */ }

    // Returns false if full. Lock-free: no mutex, no blocking.
    template <typename... Args>
    bool try_emplace(Args&&... args) {
        ((void)args, ...);
        throw std::logic_error("TODO: try_emplace");
    }

    bool try_push(const T& v) { return try_emplace(v); }
    bool try_push(T&& v) { return try_emplace(std::move(v)); }

    // Returns false if empty.
    bool try_pop(T& out) {
        (void)out;
        throw std::logic_error("TODO: try_pop");
    }

    std::size_t size_approx() const noexcept { return 0; /* TODO */ }

private:
    // TODO: an array of cells, each holding a sequence number and storage
    // for one T; an enqueue position and a dequeue position, each on its
    // own cache line.
};

}  // namespace hft
