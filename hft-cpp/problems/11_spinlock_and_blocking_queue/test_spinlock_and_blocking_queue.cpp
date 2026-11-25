#include "spinlock_and_blocking_queue.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "testing.hpp"

using hft::BlockingQueue;
using hft::Spinlock;
using namespace std::chrono_literals;

// ---------------------------------------------------------------- Spinlock

TEST(spinlock_basic) {
    Spinlock s;
    CHECK(s.try_lock());
    CHECK_FALSE(s.try_lock());
    s.unlock();
    s.lock();
    CHECK_FALSE(s.try_lock());
    s.unlock();
    CHECK(s.try_lock());
    s.unlock();
    CHECK(sizeof(Spinlock) <= 8u);
}

TEST(spinlock_works_with_std_guards) {
    Spinlock s;
    {
        std::lock_guard<Spinlock> g(s);
        CHECK_FALSE(s.try_lock());
    }
    {
        std::unique_lock<Spinlock> g(s, std::try_to_lock);
        CHECK(g.owns_lock());
    }
    CHECK(s.try_lock());
    s.unlock();
}

TEST(spinlock_mutual_exclusion) {
    Spinlock s;
    { std::lock_guard g(s); }  // fail fast (single-threaded) if unimplemented
    // A plain, non-atomic counter: only correct if the lock provides mutual
    // exclusion AND happens-before (TSan checks the latter).
    std::uint64_t counter = 0;
    std::vector<std::thread> threads;
    constexpr int kThreads = 4, kIters = 50000;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&] {
            for (int i = 0; i < kIters; ++i) {
                std::lock_guard g(s);
                ++counter;
            }
        });
    }
    for (auto& t : threads) t.join();
    CHECK_EQ(counter, std::uint64_t(kThreads) * kIters);
}

TEST(spinlock_protects_non_trivial_state) {
    Spinlock s;
    { std::lock_guard g(s); }
    std::vector<int> shared;
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < 10000; ++i) {
                std::lock_guard g(s);
                shared.push_back(t);
            }
        });
    }
    for (auto& t : threads) t.join();
    CHECK_EQ(shared.size(), 40000u);
}

// ----------------------------------------------------------- BlockingQueue

TEST(queue_fifo_single_thread) {
    BlockingQueue<int> q(8);
    CHECK_EQ(q.capacity(), 8u);
    for (int i = 0; i < 5; ++i) CHECK(q.push(i));
    CHECK_EQ(q.size(), 5u);
    for (int i = 0; i < 5; ++i) CHECK_EQ(*q.pop(), i);
    CHECK_FALSE(q.try_pop().has_value());
}

TEST(queue_try_push_respects_capacity) {
    BlockingQueue<int> q(2);
    CHECK(q.try_push(1));
    CHECK(q.try_push(2));
    CHECK_FALSE(q.try_push(3));
    CHECK_EQ(*q.try_pop(), 1);
    CHECK(q.try_push(3));
    CHECK_EQ(q.size(), 2u);
}

TEST(queue_try_push_failure_keeps_value) {
    BlockingQueue<std::unique_ptr<int>> q(1);
    CHECK(q.try_push(std::make_unique<int>(1)));
    auto p = std::make_unique<int>(2);
    CHECK_FALSE(q.try_push(std::move(p)));
    CHECK(p != nullptr);  // not moved from on failure
    CHECK_EQ(*p, 2);
}

TEST(queue_move_only_values) {
    BlockingQueue<std::unique_ptr<std::string>> q(4);
    q.push(std::make_unique<std::string>("a"));
    auto v = q.pop();
    CHECK(v.has_value());
    CHECK_EQ(**v, std::string("a"));
}

TEST(queue_pop_for_times_out) {
    BlockingQueue<int> q(1);
    auto t0 = std::chrono::steady_clock::now();
    auto v = q.pop_for(30ms);
    auto elapsed = std::chrono::steady_clock::now() - t0;
    CHECK_FALSE(v.has_value());
    CHECK(elapsed >= 25ms);
    CHECK(elapsed < 2s);
}

TEST(queue_close_semantics) {
    BlockingQueue<int> q(4);
    q.push(1);
    q.push(2);
    q.close();
    CHECK(q.closed());
    CHECK_FALSE(q.push(3));
    CHECK_FALSE(q.try_push(3));
    // Remaining items still drain, then pop returns nullopt without blocking.
    CHECK_EQ(*q.pop(), 1);
    CHECK_EQ(*q.pop(), 2);
    CHECK_FALSE(q.pop().has_value());
}

TEST(queue_close_wakes_blocked_consumers) {
    BlockingQueue<int> q(4);
    q.try_pop();  // fail fast if unimplemented
    std::atomic<int> woke{0};
    std::vector<std::thread> consumers;
    for (int i = 0; i < 3; ++i)
        consumers.emplace_back([&] {
            if (!q.pop().has_value()) ++woke;
        });
    std::this_thread::sleep_for(20ms);
    q.close();
    for (auto& t : consumers) t.join();
    CHECK_EQ(woke.load(), 3);
}

TEST(queue_close_wakes_blocked_producer) {
    BlockingQueue<int> q(1);
    q.push(1);
    std::atomic<int> result{-1};
    std::thread producer([&] { result = q.push(2) ? 1 : 0; });
    std::this_thread::sleep_for(20ms);
    q.close();
    producer.join();
    CHECK_EQ(result.load(), 0);
}

TEST(queue_full_push_blocks_until_pop) {
    BlockingQueue<int> q(1);
    q.push(1);
    std::atomic<bool> pushed{false};
    std::thread producer([&] {
        q.push(2);
        pushed = true;
    });
    std::this_thread::sleep_for(20ms);
    CHECK_FALSE(pushed.load());
    CHECK_EQ(*q.pop(), 1);
    producer.join();
    CHECK(pushed.load());
    CHECK_EQ(*q.pop(), 2);
}

TEST(queue_many_producers_many_consumers) {
    BlockingQueue<std::uint64_t> q(64);
    q.try_pop();
    constexpr int kProducers = 4, kConsumers = 4, kPerProducer = 20000;
    std::atomic<std::uint64_t> sum{0}, count{0};
    std::vector<std::thread> threads;
    for (int c = 0; c < kConsumers; ++c)
        threads.emplace_back([&] {
            while (auto v = q.pop()) {
                sum += *v;
                ++count;
            }
        });
    std::vector<std::thread> producers;
    for (int p = 0; p < kProducers; ++p)
        producers.emplace_back([&, p] {
            for (int i = 1; i <= kPerProducer; ++i) q.push(std::uint64_t(p) * 1000000 + i);
        });
    for (auto& t : producers) t.join();
    q.close();
    for (auto& t : threads) t.join();
    std::uint64_t expected = 0;
    for (int p = 0; p < kProducers; ++p)
        for (int i = 1; i <= kPerProducer; ++i) expected += std::uint64_t(p) * 1000000 + i;
    CHECK_EQ(count.load(), std::uint64_t(kProducers) * kPerProducer);
    CHECK_EQ(sum.load(), expected);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
