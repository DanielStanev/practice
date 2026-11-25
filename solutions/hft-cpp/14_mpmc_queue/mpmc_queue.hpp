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
    struct alignas(64) Cell {
        std::atomic<std::size_t> seq;
        alignas(T) std::byte storage[sizeof(T)];
        T* ptr() noexcept { return std::launder(reinterpret_cast<T*>(storage)); }
    };

public:
    explicit MpmcQueue(std::size_t capacity) : mask_(capacity - 1) {
        if (capacity < 2 || (capacity & (capacity - 1)) != 0)
            throw std::invalid_argument("capacity must be a power of two >= 2");
        cells_ = static_cast<Cell*>(::operator new(sizeof(Cell) * capacity, std::align_val_t{alignof(Cell)}));
        for (std::size_t i = 0; i < capacity; ++i) ::new (&cells_[i]) Cell{{i}, {}};
    }

    MpmcQueue(const MpmcQueue&) = delete;
    MpmcQueue& operator=(const MpmcQueue&) = delete;

    ~MpmcQueue() {
        // Destroy anything still queued.
        std::size_t head = dequeue_pos_.load(std::memory_order_relaxed);
        std::size_t tail = enqueue_pos_.load(std::memory_order_relaxed);
        for (std::size_t p = head; p != tail; ++p) std::destroy_at(cells_[p & mask_].ptr());
        for (std::size_t i = 0; i <= mask_; ++i) cells_[i].~Cell();
        ::operator delete(cells_, std::align_val_t{alignof(Cell)});
    }

    std::size_t capacity() const noexcept { return mask_ + 1; }

    template <typename... Args>
    bool try_emplace(Args&&... args) {
        std::size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            Cell& c = cells_[pos & mask_];
            std::size_t seq = c.seq.load(std::memory_order_acquire);
            auto diff = static_cast<std::intptr_t>(seq) - static_cast<std::intptr_t>(pos);
            if (diff == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    ::new (static_cast<void*>(c.storage)) T(std::forward<Args>(args)...);
                    c.seq.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;  // full
            } else {
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

    bool try_push(const T& v) { return try_emplace(v); }
    bool try_push(T&& v) { return try_emplace(std::move(v)); }

    bool try_pop(T& out) {
        std::size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            Cell& c = cells_[pos & mask_];
            std::size_t seq = c.seq.load(std::memory_order_acquire);
            auto diff = static_cast<std::intptr_t>(seq) - static_cast<std::intptr_t>(pos + 1);
            if (diff == 0) {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    out = std::move(*c.ptr());
                    std::destroy_at(c.ptr());
                    c.seq.store(pos + mask_ + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;  // empty
            } else {
                pos = dequeue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

    std::size_t size_approx() const noexcept {
        std::size_t t = enqueue_pos_.load(std::memory_order_relaxed);
        std::size_t h = dequeue_pos_.load(std::memory_order_relaxed);
        return t >= h ? t - h : 0;
    }

private:
    Cell* cells_;
    const std::size_t mask_;
    alignas(64) std::atomic<std::size_t> enqueue_pos_{0};
    alignas(64) std::atomic<std::size_t> dequeue_pos_{0};
};

}  // namespace hft
