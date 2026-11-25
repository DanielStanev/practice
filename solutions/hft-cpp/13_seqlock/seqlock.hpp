#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace hft {

template <typename T>
class Seqlock {
    static_assert(std::is_trivially_copyable_v<T>, "Seqlock<T> requires a trivially copyable T");
    static constexpr std::size_t kWords = (sizeof(T) + 7) / 8;

public:
    Seqlock() noexcept : Seqlock(T{}) {}
    explicit Seqlock(const T& initial) noexcept { write_words(initial); }

    void store(const T& value) noexcept {
        const std::uint64_t s = seq_.load(std::memory_order_relaxed);
        seq_.store(s + 1, std::memory_order_relaxed);
        // Order the odd sequence number before the data writes.
        std::atomic_thread_fence(std::memory_order_release);
        write_words(value);
        // Publish: data writes happen-before the even sequence number.
        seq_.store(s + 2, std::memory_order_release);
    }

    bool try_load(T& out) const noexcept {
        const std::uint64_t s1 = seq_.load(std::memory_order_acquire);
        if (s1 & 1) return false;
        std::uint64_t buf[kWords];
        for (std::size_t i = 0; i < kWords; ++i) buf[i] = words_[i].load(std::memory_order_relaxed);
        // Order the data reads before the second sequence read.
        std::atomic_thread_fence(std::memory_order_acquire);
        const std::uint64_t s2 = seq_.load(std::memory_order_relaxed);
        if (s1 != s2) return false;
        std::memcpy(&out, buf, sizeof(T));
        return true;
    }

    T load() const noexcept {
        T out;
        while (!try_load(out)) {
#if defined(__x86_64__) || defined(__i386__)
            __builtin_ia32_pause();
#endif
        }
        return out;
    }

    std::uint64_t version() const noexcept { return seq_.load(std::memory_order_acquire) / 2; }

private:
    void write_words(const T& value) noexcept {
        std::uint64_t buf[kWords] = {};
        std::memcpy(buf, &value, sizeof(T));
        for (std::size_t i = 0; i < kWords; ++i) words_[i].store(buf[i], std::memory_order_relaxed);
    }

    alignas(64) std::atomic<std::uint64_t> seq_{0};
    std::atomic<std::uint64_t> words_[kWords];
};

}  // namespace hft
