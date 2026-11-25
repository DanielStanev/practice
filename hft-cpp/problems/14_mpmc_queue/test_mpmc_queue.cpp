#include "mpmc_queue.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "testing.hpp"

using hft::MpmcQueue;

namespace {
struct Tracked {
    static inline std::atomic<int> alive{0};
    int v = 0;
    Tracked() { ++alive; }
    explicit Tracked(int x) : v(x) { ++alive; }
    Tracked(const Tracked& o) : v(o.v) { ++alive; }
    Tracked(Tracked&& o) noexcept : v(o.v) { ++alive; }
    Tracked& operator=(const Tracked&) = default;
    Tracked& operator=(Tracked&&) noexcept = default;
    ~Tracked() { --alive; }
};
}  // namespace

TEST(rejects_bad_capacity) {
    CHECK_THROWS_AS(MpmcQueue<int>(0), std::invalid_argument);
    CHECK_THROWS_AS(MpmcQueue<int>(1), std::invalid_argument);
    CHECK_THROWS_AS(MpmcQueue<int>(12), std::invalid_argument);
    MpmcQueue<int> ok(16);
    CHECK_EQ(ok.capacity(), 16u);
}

TEST(fifo_single_thread) {
    MpmcQueue<int> q(8);
    for (int i = 0; i < 8; ++i) CHECK(q.try_push(i));
    CHECK_FALSE(q.try_push(8));  // exactly capacity elements fit
    CHECK_EQ(q.size_approx(), 8u);
    int out;
    for (int i = 0; i < 8; ++i) {
        CHECK(q.try_pop(out));
        CHECK_EQ(out, i);
    }
    CHECK_FALSE(q.try_pop(out));
}

TEST(wraps_around) {
    MpmcQueue<std::uint64_t> q(4);
    std::uint64_t out;
    for (std::uint64_t i = 0; i < 100000; ++i) {
        CHECK(q.try_push(i));
        CHECK(q.try_pop(out));
        CHECK_EQ(out, i);
    }
}

TEST(move_only_and_strings) {
    MpmcQueue<std::unique_ptr<std::string>> q(4);
    CHECK(q.try_push(std::make_unique<std::string>("abc")));
    std::unique_ptr<std::string> out;
    CHECK(q.try_pop(out));
    CHECK_EQ(*out, std::string("abc"));
}

TEST(destroys_remaining_elements) {
    Tracked::alive = 0;
    {
        MpmcQueue<Tracked> q(8);
        for (int i = 0; i < 6; ++i) q.try_emplace(i);
        Tracked out;
        q.try_pop(out);
        CHECK_EQ(out.v, 0);
        CHECK_EQ(Tracked::alive.load(), 6);  // 5 queued + out
    }
    CHECK_EQ(Tracked::alive.load(), 0);
}

TEST(many_producers_many_consumers_exactly_once) {
    constexpr int kProducers = 4, kConsumers = 4;
    const std::uint64_t kPerProducer = testing::stress(250'000);
    MpmcQueue<std::uint64_t> q(1024);
    std::uint64_t probe;
    q.try_pop(probe);  // fail fast if unimplemented
    std::vector<std::atomic<std::uint8_t>> seen(kProducers * kPerProducer);
    std::atomic<std::uint64_t> consumed{0};
    std::atomic<bool> duplicate{false};

    std::vector<std::thread> threads;
    for (int p = 0; p < kProducers; ++p)
        threads.emplace_back([&, p] {
            for (std::uint64_t i = 0; i < kPerProducer; ++i) {
                std::uint64_t v = std::uint64_t(p) * kPerProducer + i;
                while (!q.try_push(v)) std::this_thread::yield();
            }
        });
    for (int c = 0; c < kConsumers; ++c)
        threads.emplace_back([&] {
            std::uint64_t v;
            while (consumed.load(std::memory_order_relaxed) < kProducers * kPerProducer) {
                if (q.try_pop(v)) {
                    if (seen[v].fetch_add(1) != 0) duplicate = true;
                    consumed.fetch_add(1);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    for (auto& t : threads) t.join();
    CHECK_FALSE(duplicate.load());
    CHECK_EQ(consumed.load(), kProducers * kPerProducer);
    for (auto& s : seen) CHECK_EQ(s.load(), 1u);
}

TEST(per_producer_order_is_preserved) {
    // With a single consumer, items from any one producer arrive in the
    // order that producer pushed them.
    constexpr int kProducers = 3;
    const std::uint64_t kPerProducer = testing::stress(200'000);
    MpmcQueue<std::uint64_t> q(256);
    std::uint64_t probe;
    q.try_pop(probe);
    std::vector<std::thread> producers;
    for (int p = 0; p < kProducers; ++p)
        producers.emplace_back([&, p] {
            for (std::uint64_t i = 0; i < kPerProducer; ++i)
                while (!q.try_push((std::uint64_t(p) << 32) | i)) std::this_thread::yield();
        });
    std::uint64_t next[kProducers] = {};
    bool in_order = true;
    for (std::uint64_t got = 0; got < kProducers * kPerProducer;) {
        std::uint64_t v;
        if (!q.try_pop(v)) continue;
        ++got;
        auto p = v >> 32;
        auto i = v & 0xffffffffu;
        if (i != next[p]) in_order = false;
        next[p] = i + 1;
    }
    for (auto& t : producers) t.join();
    CHECK(in_order);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
