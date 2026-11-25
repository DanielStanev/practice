#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace hft {

inline void cpu_relax() noexcept {
#if defined(__x86_64__) || defined(__i386__)
    _mm_pause();
#elif defined(__aarch64__)
    asm volatile("yield" ::: "memory");
#endif
}

class Spinlock {
public:
    void lock() noexcept {
        for (;;) {
            if (!locked_.exchange(true, std::memory_order_acquire)) return;
            while (locked_.load(std::memory_order_relaxed)) cpu_relax();
        }
    }
    bool try_lock() noexcept {
        return !locked_.load(std::memory_order_relaxed) &&
               !locked_.exchange(true, std::memory_order_acquire);
    }
    void unlock() noexcept { locked_.store(false, std::memory_order_release); }

private:
    std::atomic<bool> locked_{false};
};

template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(std::size_t capacity) : capacity_(capacity) {}

    bool push(T value) {
        std::unique_lock lk(m_);
        not_full_.wait(lk, [&] { return closed_ || q_.size() < capacity_; });
        if (closed_) return false;
        q_.push_back(std::move(value));
        lk.unlock();
        not_empty_.notify_one();
        return true;
    }

    template <typename U>
    bool try_push(U&& value) {
        {
            std::lock_guard lk(m_);
            if (closed_ || q_.size() >= capacity_) return false;
            q_.push_back(std::forward<U>(value));
        }
        not_empty_.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock lk(m_);
        not_empty_.wait(lk, [&] { return closed_ || !q_.empty(); });
        return take(lk);
    }

    std::optional<T> try_pop() {
        std::unique_lock lk(m_);
        return take(lk);
    }

    template <typename Rep, typename Period>
    std::optional<T> pop_for(std::chrono::duration<Rep, Period> timeout) {
        std::unique_lock lk(m_);
        not_empty_.wait_for(lk, timeout, [&] { return closed_ || !q_.empty(); });
        return take(lk);
    }

    void close() {
        {
            std::lock_guard lk(m_);
            closed_ = true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }

    bool closed() const {
        std::lock_guard lk(m_);
        return closed_;
    }

    std::size_t size() const {
        std::lock_guard lk(m_);
        return q_.size();
    }

    std::size_t capacity() const noexcept { return capacity_; }

private:
    std::optional<T> take(std::unique_lock<std::mutex>& lk) {
        if (q_.empty()) return std::nullopt;
        std::optional<T> v(std::move(q_.front()));
        q_.pop_front();
        lk.unlock();
        not_full_.notify_one();
        return v;
    }

    mutable std::mutex m_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::deque<T> q_;
    std::size_t capacity_;
    bool closed_ = false;
};

}  // namespace hft
