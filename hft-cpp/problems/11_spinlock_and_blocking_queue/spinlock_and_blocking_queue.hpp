// Problem 11: Spinlock and a bounded blocking queue (the baseline)
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace hft {

// Hint to the CPU that we are in a spin-wait loop (x86: PAUSE).
inline void cpu_relax() noexcept {
    // TODO
}

// Test-and-test-and-set spinlock. Satisfies the Lockable requirements so it
// works with std::lock_guard / std::unique_lock / std::scoped_lock.
class Spinlock {
public:
    void lock() { throw std::logic_error("TODO: Spinlock::lock"); }
    bool try_lock() { throw std::logic_error("TODO: Spinlock::try_lock"); }
    void unlock() { throw std::logic_error("TODO: Spinlock::unlock"); }

private:
    // TODO: one atomic flag. sizeof(Spinlock) <= 8.
};

// Bounded multi-producer multi-consumer queue built on std::mutex and
// std::condition_variable.
template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(std::size_t capacity) : capacity_(capacity) {}

    // Blocks while full. Returns false (and drops value) if the queue is or
    // becomes closed.
    bool push(T value) {
        (void)value;
        throw std::logic_error("TODO: push");
    }

    // Never blocks. Returns false if full or closed; `value` is only moved
    // from on success.
    template <typename U>
    bool try_push(U&& value) {
        (void)value;
        throw std::logic_error("TODO: try_push");
    }

    // Blocks while empty. Returns nullopt once the queue is closed and drained.
    std::optional<T> pop() { throw std::logic_error("TODO: pop"); }

    // Never blocks.
    std::optional<T> try_pop() { throw std::logic_error("TODO: try_pop"); }

    // Blocks at most `timeout`.
    template <typename Rep, typename Period>
    std::optional<T> pop_for(std::chrono::duration<Rep, Period> timeout) {
        (void)timeout;
        throw std::logic_error("TODO: pop_for");
    }

    // Wakes every blocked producer and consumer. Items already queued can
    // still be popped.
    void close() { throw std::logic_error("TODO: close"); }

    bool closed() const { throw std::logic_error("TODO: closed"); }
    std::size_t size() const { throw std::logic_error("TODO: size"); }
    std::size_t capacity() const noexcept { return capacity_; }

private:
    std::size_t capacity_;
    // TODO: mutex, two condition variables, storage, closed flag.
};

}  // namespace hft
