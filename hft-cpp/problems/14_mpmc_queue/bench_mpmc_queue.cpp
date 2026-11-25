// ./run_tests.sh bench
// Throughput with N producers and N consumers. Compare with BlockingQueue
// (problem 11) and, for 1x1, with SpscQueue (problem 12).
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>

#include "bench.hpp"
#include "mpmc_queue.hpp"

int main() {
    constexpr std::uint64_t kTotal = 4'000'000;
    for (int n : {1, 2}) {
        char name[64];
        std::snprintf(name, sizeof name, "MpmcQueue %dP x %dC", n, n);
        bench::run(name, kTotal, [&] {
            hft::MpmcQueue<std::uint64_t> q(4096);
            std::atomic<std::uint64_t> consumed{0};
            std::vector<std::thread> ts;
            for (int p = 0; p < n; ++p)
                ts.emplace_back([&] {
                    for (std::uint64_t i = 0; i < kTotal / n; ++i)
                        while (!q.try_push(i)) {
                        }
                });
            for (int c = 0; c < n; ++c)
                ts.emplace_back([&] {
                    std::uint64_t v;
                    while (consumed.load(std::memory_order_relaxed) < kTotal)
                        if (q.try_pop(v)) consumed.fetch_add(1, std::memory_order_relaxed);
                });
            for (auto& t : ts) t.join();
        }, 3);
    }
}
