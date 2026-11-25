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

class LatencyHistogram {
public:
    explicit LatencyHistogram(std::uint64_t max_value = 1'000'000'000, int sub_bucket_bits = 7)
        : max_value_(max_value), bits_(sub_bucket_bits) {
        if (sub_bucket_bits < 1 || sub_bucket_bits > 16)
            throw std::invalid_argument("sub_bucket_bits must be in [1, 16]");
        counts_.assign(index_of(max_value) + 1, 0);
    }

    void record(std::uint64_t value) noexcept { record_n(value, 1); }

    void record_n(std::uint64_t value, std::uint64_t n) noexcept {
        if (n == 0) return;
        if (value > max_value_) {
            overflow_ += n;
            value = max_value_;
        }
        counts_[index_of(value)] += n;
        count_ += n;
        sum_ += static_cast<long double>(value) * n;
        min_ = std::min(min_, value);
        max_ = std::max(max_, value);
    }

    std::uint64_t count() const noexcept { return count_; }
    std::uint64_t min() const noexcept { return count_ ? min_ : 0; }
    std::uint64_t max() const noexcept { return max_; }
    double mean() const noexcept {
        return count_ ? static_cast<double>(sum_ / static_cast<long double>(count_)) : 0.0;
    }
    std::uint64_t overflow_count() const noexcept { return overflow_; }
    std::size_t bucket_count() const noexcept { return counts_.size(); }

    std::uint64_t percentile(double p) const {
        if (count_ == 0) return 0;
        if (p <= 0.0) return min();
        if (p >= 100.0) return max();
        auto rank = static_cast<std::uint64_t>(std::ceil(p / 100.0 * static_cast<double>(count_)));
        rank = std::clamp<std::uint64_t>(rank, 1, count_);
        std::uint64_t seen = 0;
        for (std::size_t i = 0; i < counts_.size(); ++i) {
            seen += counts_[i];
            if (seen >= rank) return std::clamp(highest_in(i), min(), max());
        }
        return max();
    }

    void reset() noexcept {
        std::fill(counts_.begin(), counts_.end(), 0);
        count_ = overflow_ = 0;
        sum_ = 0;
        min_ = std::numeric_limits<std::uint64_t>::max();
        max_ = 0;
    }

    void merge(const LatencyHistogram& o) {
        if (o.max_value_ != max_value_ || o.bits_ != bits_)
            throw std::invalid_argument("histogram configurations differ");
        for (std::size_t i = 0; i < counts_.size(); ++i) counts_[i] += o.counts_[i];
        count_ += o.count_;
        overflow_ += o.overflow_;
        sum_ += o.sum_;
        if (o.count_) {
            min_ = std::min(min_, o.min_);
            max_ = std::max(max_, o.max_);
        }
    }

    std::string summary() const {
        auto f = [](std::uint64_t v) { return std::to_string(v); };
        return "count=" + f(count()) + " min=" + f(min()) + " p50=" + f(percentile(50)) +
               " p90=" + f(percentile(90)) + " p99=" + f(percentile(99)) +
               " p99.9=" + f(percentile(99.9)) + " max=" + f(max());
    }

private:
    std::size_t index_of(std::uint64_t v) const noexcept {
        const std::uint64_t sub = std::uint64_t{1} << bits_;
        if (v < 2 * sub) return static_cast<std::size_t>(v);
        int msb = 63 - std::countl_zero(v);
        int shift = msb - bits_;
        return static_cast<std::size_t>(static_cast<std::uint64_t>(shift) * sub + (v >> shift));
    }

    std::uint64_t highest_in(std::size_t idx) const noexcept {
        const std::uint64_t sub = std::uint64_t{1} << bits_;
        if (idx < 2 * sub) return idx;
        std::uint64_t shift = idx / sub - 1;
        std::uint64_t mantissa = idx - shift * sub;
        return (mantissa << shift) + ((std::uint64_t{1} << shift) - 1);
    }

    std::uint64_t max_value_;
    int bits_;
    std::vector<std::uint64_t> counts_;
    std::uint64_t count_ = 0;
    std::uint64_t overflow_ = 0;
    long double sum_ = 0;
    std::uint64_t min_ = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t max_ = 0;
};

inline std::uint64_t steady_now_ns() noexcept {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                          std::chrono::steady_clock::now().time_since_epoch())
                                          .count());
}

inline std::uint64_t read_tsc() noexcept {
#if defined(__x86_64__) || defined(__i386__)
    return __rdtsc();
#else
    return steady_now_ns();
#endif
}

class TscClock {
public:
    void calibrate(std::chrono::milliseconds duration = std::chrono::milliseconds(20)) {
        std::uint64_t t0 = steady_now_ns();
        std::uint64_t c0 = read_tsc();
        std::uint64_t deadline = t0 + static_cast<std::uint64_t>(
                                          std::chrono::nanoseconds(duration).count());
        while (steady_now_ns() < deadline) {
        }
        std::uint64_t t1 = steady_now_ns();
        std::uint64_t c1 = read_tsc();
        ns_per_tick_ = static_cast<double>(t1 - t0) / static_cast<double>(c1 - c0);
    }
    double ns_per_tick() const noexcept { return ns_per_tick_; }
    std::uint64_t to_ns(std::uint64_t ticks) const noexcept {
        return static_cast<std::uint64_t>(static_cast<double>(ticks) * ns_per_tick_);
    }

private:
    double ns_per_tick_ = 0.0;
};

template <typename F>
void measure(LatencyHistogram& hist, std::size_t iterations, F&& f) {
    for (std::size_t i = 0; i < iterations; ++i) {
        std::uint64_t t0 = steady_now_ns();
        f();
        std::uint64_t t1 = steady_now_ns();
        hist.record(t1 - t0);
    }
}

}  // namespace hft
