// Problem 12: lock-free single-producer single-consumer ring buffer
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
//
// Threading contract: exactly one thread calls the producer functions
// (try_push / try_emplace) and exactly one other thread calls the consumer
// functions (front / pop / try_pop). size_approx() and empty() may be
// called from either.
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hft {

inline constexpr std::size_t kCacheLine = 64;

template <typename T, std::size_t Capacity>
class SpscQueue {  // TODO: alignment
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of two >= 2");

public:
    SpscQueue() = default;
    // TODO: non-copyable

    // Destroys any elements still in the queue.
    ~SpscQueue() {
        // TODO
    }

    static constexpr std::size_t capacity() noexcept { return Capacity; }

    // Producer. Constructs in place. Returns false if full (exactly
    // Capacity elements fit). Never allocates, never blocks.
    template <typename... Args>
    bool try_emplace(Args&&... args) {
        ((void)args, ...);
        throw std::logic_error("TODO: try_emplace");
    }

    bool try_push(const T& v) { return try_emplace(v); }
    bool try_push(T&& v) { return try_emplace(std::move(v)); }

    // Consumer. Pointer to the oldest element, or nullptr if empty. Lets the
    // consumer process an element in place without copying it out.
    T* front() { throw std::logic_error("TODO: front"); }

    // Consumer. Destroys the oldest element. Precondition: front() != nullptr.
    void pop() { throw std::logic_error("TODO: pop"); }

    // Consumer. Moves the oldest element into out. Returns false if empty.
    bool try_pop(T& out) {
        (void)out;
        throw std::logic_error("TODO: try_pop");
    }

    std::size_t size_approx() const { throw std::logic_error("TODO: size_approx"); }
    bool empty() const { return size_approx() == 0; }

private:
    // TODO: head index (written by consumer) and tail index (written by
    // producer) on separate cache lines, plus each side's cached copy of the
    // other's index, plus raw storage for Capacity elements.
};

}  // namespace hft
