#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace hft {

inline constexpr std::size_t kCacheLine = 64;

template <typename T, std::size_t Capacity>
class alignas(kCacheLine) SpscQueue {
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of two >= 2");
    static constexpr std::size_t kMask = Capacity - 1;

public:
    SpscQueue() = default;
    SpscQueue(const SpscQueue&) = delete;
    SpscQueue& operator=(const SpscQueue&) = delete;

    ~SpscQueue() {
        while (front()) pop();
    }

    static constexpr std::size_t capacity() noexcept { return Capacity; }

    template <typename... Args>
    bool try_emplace(Args&&... args) {
        const std::uint64_t tail = tail_.load(std::memory_order_relaxed);
        if (tail - head_cache_ == Capacity) {
            head_cache_ = head_.load(std::memory_order_acquire);
            if (tail - head_cache_ == Capacity) return false;
        }
        ::new (static_cast<void*>(slot(tail))) T(std::forward<Args>(args)...);
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    bool try_push(const T& v) { return try_emplace(v); }
    bool try_push(T&& v) { return try_emplace(std::move(v)); }

    T* front() noexcept {
        const std::uint64_t head = head_.load(std::memory_order_relaxed);
        if (head == tail_cache_) {
            tail_cache_ = tail_.load(std::memory_order_acquire);
            if (head == tail_cache_) return nullptr;
        }
        return std::launder(reinterpret_cast<T*>(slot(head)));
    }

    void pop() noexcept {
        const std::uint64_t head = head_.load(std::memory_order_relaxed);
        std::destroy_at(std::launder(reinterpret_cast<T*>(slot(head))));
        head_.store(head + 1, std::memory_order_release);
    }

    bool try_pop(T& out) {
        T* f = front();
        if (!f) return false;
        out = std::move(*f);
        pop();
        return true;
    }

    std::size_t size_approx() const noexcept {
        std::uint64_t t = tail_.load(std::memory_order_acquire);
        std::uint64_t h = head_.load(std::memory_order_acquire);
        return t >= h ? static_cast<std::size_t>(t - h) : 0;
    }
    bool empty() const noexcept { return size_approx() == 0; }

private:
    std::byte* slot(std::uint64_t i) noexcept { return storage_ + (i & kMask) * sizeof(T); }

    // Consumer-owned line.
    alignas(kCacheLine) std::atomic<std::uint64_t> head_{0};
    std::uint64_t tail_cache_ = 0;
    // Producer-owned line.
    alignas(kCacheLine) std::atomic<std::uint64_t> tail_{0};
    std::uint64_t head_cache_ = 0;
    alignas(kCacheLine) alignas(T) std::byte storage_[Capacity * sizeof(T)];
};

}  // namespace hft
