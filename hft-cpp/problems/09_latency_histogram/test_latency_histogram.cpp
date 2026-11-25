#include "latency_histogram.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <random>
#include <stdexcept>
#include <vector>

#include "alloc_counter.hpp"
#include "testing.hpp"

using hft::LatencyHistogram;


namespace {

// Nearest-rank percentile on sorted data: the smallest value such that at
// least p% of samples are <= it.
std::uint64_t exact_percentile(const std::vector<std::uint64_t>& sorted, double p) {
    if (p <= 0) return sorted.front();
    auto rank = static_cast<std::size_t>(std::ceil(p / 100.0 * double(sorted.size())));
    rank = std::clamp<std::size_t>(rank, 1, sorted.size());
    return sorted[rank - 1];
}

// Reported value must be >= exact and within the configured relative error.
void check_percentile(const LatencyHistogram& h, const std::vector<std::uint64_t>& sorted,
                      double p, int bits) {
    std::uint64_t exact = exact_percentile(sorted, p);
    std::uint64_t got = h.percentile(p);
    double tolerance = double(exact) / double(1u << bits) + 1.0;
    if (got < exact || double(got - exact) > tolerance) {
        ::testing::fail(__FILE__, __LINE__,
                        "percentile " + std::to_string(p) + ": exact " + std::to_string(exact) +
                            " got " + std::to_string(got));
    }
}

}  // namespace

TEST(empty_histogram) {
    LatencyHistogram h;
    CHECK_EQ(h.count(), 0u);
    CHECK_EQ(h.percentile(50), 0u);
    CHECK_EQ(h.min(), 0u);
    CHECK_EQ(h.max(), 0u);
    CHECK_EQ(h.mean(), 0.0);
}

TEST(small_values_are_exact) {
    LatencyHistogram h(1'000'000, 7);
    for (std::uint64_t v = 0; v < 256; ++v) h.record(v);
    CHECK_EQ(h.count(), 256u);
    CHECK_EQ(h.min(), 0u);
    CHECK_EQ(h.max(), 255u);
    CHECK_EQ(h.percentile(50), 127u);
    CHECK_EQ(h.percentile(100), 255u);
    CHECK_EQ(h.percentile(0), 0u);
    CHECK_NEAR(h.mean(), 127.5, 1e-9);
}

TEST(bucket_count_is_logarithmic) {
    // 1 second range in ns with ~0.8% precision needs a few thousand
    // counters, not a billion.
    LatencyHistogram h(1'000'000'000, 7);
    CHECK(h.bucket_count() <= 4096u);
    CHECK(h.bucket_count() >= 1000u);
    LatencyHistogram coarse(1'000'000'000, 3);
    CHECK(coarse.bucket_count() < h.bucket_count());
}

TEST(percentiles_within_relative_error) {
    for (int bits : {3, 5, 7, 10}) {
        LatencyHistogram h(10'000'000'000ull, bits);
        std::mt19937_64 rng(bits);
        std::lognormal_distribution<double> dist(8.0, 1.5);  // median ~3us, long tail
        std::vector<std::uint64_t> samples;
        for (int i = 0; i < 200000; ++i) {
            auto v = static_cast<std::uint64_t>(dist(rng));
            samples.push_back(v);
            h.record(v);
        }
        std::sort(samples.begin(), samples.end());
        for (double p : {0.0, 1.0, 25.0, 50.0, 90.0, 99.0, 99.9, 99.99, 100.0})
            check_percentile(h, samples, p, bits);
        CHECK_EQ(h.min(), samples.front());
        CHECK_EQ(h.max(), samples.back());
    }
}

TEST(mean_is_exact) {
    LatencyHistogram h;
    std::uint64_t sum = 0;
    for (std::uint64_t v = 1000; v < 200000; v += 37) {
        h.record(v);
        sum += v;
    }
    CHECK_NEAR(h.mean(), double(sum) / double(h.count()), 1e-6);
}

TEST(record_n) {
    LatencyHistogram h;
    h.record_n(100, 99);
    h.record_n(5000, 1);
    CHECK_EQ(h.count(), 100u);
    CHECK_EQ(h.percentile(99), 100u);
    CHECK(h.percentile(99.5) >= 5000u);
    CHECK_NEAR(h.mean(), (100.0 * 99 + 5000) / 100, 1e-9);
}

TEST(values_above_max_are_clamped_and_counted) {
    LatencyHistogram h(1000, 7);
    h.record(10);
    h.record(5000);
    h.record(1'000'000);
    CHECK_EQ(h.count(), 3u);
    CHECK_EQ(h.overflow_count(), 2u);
    CHECK_EQ(h.max(), 1000u);
    CHECK_EQ(h.percentile(100), 1000u);
}

TEST(huge_values_do_not_overflow) {
    LatencyHistogram h(~0ull, 7);
    h.record(~0ull);
    h.record(1ull << 63);
    h.record(1);
    CHECK_EQ(h.max(), ~0ull);
    CHECK(h.percentile(50) >= (1ull << 63));
}

TEST(merge_equals_combined) {
    LatencyHistogram a, b, both;
    std::mt19937_64 rng(1);
    for (int i = 0; i < 50000; ++i) {
        std::uint64_t v = rng() % 1'000'000;
        (i % 3 ? a : b).record(v);
        both.record(v);
    }
    a.merge(b);
    CHECK_EQ(a.count(), both.count());
    CHECK_EQ(a.min(), both.min());
    CHECK_EQ(a.max(), both.max());
    for (double p : {10.0, 50.0, 99.0, 99.9}) CHECK_EQ(a.percentile(p), both.percentile(p));
    LatencyHistogram other(1000, 4);
    CHECK_THROWS_AS(a.merge(other), std::invalid_argument);
}

TEST(reset_clears) {
    LatencyHistogram h;
    for (int i = 0; i < 100; ++i) h.record(i * 1000);
    h.reset();
    CHECK_EQ(h.count(), 0u);
    CHECK_EQ(h.percentile(99), 0u);
    h.record(7);
    CHECK_EQ(h.min(), 7u);
    CHECK_EQ(h.max(), 7u);
}

TEST(record_does_not_allocate) {
    LatencyHistogram h;
    long before = alloc_counter::count();
    for (std::uint64_t i = 0; i < 1'000'000; ++i) h.record(i * 997);
    CHECK_EQ(alloc_counter::count(), before);
}

TEST(measure_records_each_iteration) {
    LatencyHistogram h;
    std::uint64_t spins = 0;
    hft::measure(h, 200, [&] {
        // ~2 microseconds of busy work
        std::uint64_t until = hft::steady_now_ns() + 2000;
        while (hft::steady_now_ns() < until) ++spins;
    });
    CHECK_EQ(h.count(), 200u);
    CHECK(h.percentile(50) >= 2000u);
    CHECK(h.percentile(50) < 10'000'000u);
}

TEST(tsc_clock_calibrates) {
    hft::TscClock clock;
    clock.calibrate(std::chrono::milliseconds(20));
    CHECK(clock.ns_per_tick() > 0.0);
    std::uint64_t t0 = hft::steady_now_ns();
    std::uint64_t c0 = hft::read_tsc();
    std::uint64_t until = t0 + 5'000'000;  // 5 ms
    while (hft::steady_now_ns() < until) {
    }
    std::uint64_t c1 = hft::read_tsc();
    std::uint64_t t1 = hft::steady_now_ns();
    double ratio = double(clock.to_ns(c1 - c0)) / double(t1 - t0);
    CHECK(ratio > 0.8);
    CHECK(ratio < 1.25);
}

TEST(summary_mentions_percentiles) {
    LatencyHistogram h;
    for (int i = 1; i <= 1000; ++i) h.record(i);
    std::string s = h.summary();
    CHECK(s.find("p50=") != std::string::npos);
    CHECK(s.find("p99=") != std::string::npos);
    CHECK(s.find("max=1000") != std::string::npos);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
