// Problem 09: latency histogram and measurement harness
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
#pragma once

#include <algorithm>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#endif

namespace hft {

// Log-linear ("HDR") histogram of non-negative integer samples, typically
// nanoseconds. Each power-of-two range is split into 2^sub_bucket_bits
// linear sub-buckets, so any reported value is within a relative error of
// 2^-sub_bucket_bits of the true value.
class LatencyHistogram {
public:
    // Samples above max_value are clamped to max_value and counted in
    // overflow_count(). Throws std::invalid_argument unless 1 <= bits <= 16.
    explicit LatencyHistogram(std::uint64_t max_value = 1'000'000'000, int sub_bucket_bits = 7) {
        (void)max_value;
        (void)sub_bucket_bits;
        // TODO: allocate all buckets here, never in record().
    }

    // O(1), no allocation, no floating point.
    void record(std::uint64_t value) noexcept { record_n(value, 1); }

    void record_n(std::uint64_t value, std::uint64_t n) noexcept {
        (void)value;
        (void)n;
        // TODO
    }

    std::uint64_t count() const noexcept { return 0; /* TODO */ }
    // Exact minimum / maximum recorded value (0 when empty).
    std::uint64_t min() const noexcept { return 0; /* TODO */ }
    std::uint64_t max() const noexcept { return 0; /* TODO */ }
    // Exact mean (0 when empty).
    double mean() const noexcept { return 0.0; /* TODO */ }
    std::uint64_t overflow_count() const noexcept { return 0; /* TODO */ }
    std::size_t bucket_count() const noexcept { return 0; /* TODO */ }

    // Nearest-rank percentile, p in [0, 100]: an upper bound for the
    // smallest recorded value v such that at least p% of samples are <= v,
    // within the configured relative error. percentile(0) == min(),
    // percentile(100) == max(). Returns 0 when empty.
    std::uint64_t percentile(double p) const {
        (void)p;
        return 0;  // TODO
    }

    void reset() noexcept {
        // TODO
    }

    // Adds o's samples. Throws std::invalid_argument if configurations differ.
    void merge(const LatencyHistogram& o) {
        (void)o;
        throw std::logic_error("TODO: merge");
    }

    // One-line human readable summary containing at least
    // "count=", "min=", "p50=", "p90=", "p99=", "p99.9=" and "max=".
    std::string summary() const {
        return "TODO";
    }
};

// Monotonic wall clock in nanoseconds.
inline std::uint64_t steady_now_ns() noexcept {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                          std::chrono::steady_clock::now().time_since_epoch())
                                          .count());
}

// Raw CPU timestamp counter (falls back to steady_now_ns on non-x86).
inline std::uint64_t read_tsc() noexcept {
    return 0;  // TODO: __rdtsc() on x86
}

// Converts TSC ticks to nanoseconds after calibrating against steady_clock.
class TscClock {
public:
    // Measure how many ns one TSC tick is by watching both clocks for `duration`.
    void calibrate(std::chrono::milliseconds duration = std::chrono::milliseconds(20)) {
        (void)duration;
        // TODO
    }
    double ns_per_tick() const noexcept { return 0.0; /* TODO */ }
    std::uint64_t to_ns(std::uint64_t ticks) const noexcept {
        (void)ticks;
        return 0;  // TODO
    }
};

// Calls f() `iterations` times and records each call's duration in ns.
template <typename F>
void measure(LatencyHistogram& hist, std::size_t iterations, F&& f) {
    (void)hist;
    (void)iterations;
    (void)f;
    // TODO
}

}  // namespace hft
