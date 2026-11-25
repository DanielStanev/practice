// ./run_tests.sh bench
// Baselines that problems 12-14 should beat.
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

#include "bench.hpp"
#include "spinlock_and_blocking_queue.hpp"

template <typename Lock>
void contended_increments(const char* name, int threads) {
    constexpr int kIters = 200'000;
    bench::run(name, std::uint64_t(kIters) * threads, [&] {
        Lock lock;
        std::uint64_t counter = 0;
        std::vector<std::thread> ts;
        for (int t = 0; t < threads; ++t)
            ts.emplace_back([&] {
                for (int i = 0; i < kIters; ++i) {
                    std::lock_guard g(lock);
                    ++counter;
                }
            });
        for (auto& t : ts) t.join();
        bench::do_not_optimize(counter);
    }, 3);
}

int main() {
    contended_increments<std::mutex>("std::mutex, 1 thread", 1);
    contended_increments<hft::Spinlock>("Spinlock, 1 thread", 1);
    contended_increments<std::mutex>("std::mutex, 4 threads", 4);
    contended_increments<hft::Spinlock>("Spinlock, 4 threads", 4);

    constexpr int kItems = 1'000'000;
    bench::run("BlockingQueue 1 producer -> 1 consumer", kItems, [&] {
        hft::BlockingQueue<std::uint64_t> q(1024);
        std::thread consumer([&] {
            std::uint64_t sum = 0;
            while (auto v = q.pop()) sum += *v;
            bench::do_not_optimize(sum);
        });
        for (int i = 0; i < kItems; ++i) q.push(std::uint64_t(i));
        q.close();
        consumer.join();
    }, 3);
}
