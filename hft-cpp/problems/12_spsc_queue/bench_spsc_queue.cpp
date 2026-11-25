// ./run_tests.sh bench
// Compare with the BlockingQueue numbers from problem 11.
// For stable numbers pin the threads to separate physical cores, e.g.
//   taskset -c 2,4 ./build/spsc_queue_bench
#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

#include "bench.hpp"
#include "spsc_queue.hpp"

int main() {
    constexpr std::uint64_t kItems = 10'000'000;
    bench::run("SpscQueue throughput, 1P -> 1C", kItems, [&] {
        auto q = std::make_unique<hft::SpscQueue<std::uint64_t, 1024>>();
        std::thread consumer([&] {
            std::uint64_t sum = 0, got = 0;
            while (got < kItems) {
                if (auto* v = q->front()) {
                    sum += *v;
                    q->pop();
                    ++got;
                }
            }
            bench::do_not_optimize(sum);
        });
        for (std::uint64_t i = 0; i < kItems; ++i)
            while (!q->try_push(i)) {
            }
        consumer.join();
    }, 3);

    // Round trip: A sends to B, B echoes back. Half of this is one-way latency.
    constexpr std::uint64_t kTrips = 1'000'000;
    bench::run("SpscQueue ping-pong round trip", kTrips, [&] {
        auto there = std::make_unique<hft::SpscQueue<std::uint64_t, 64>>();
        auto back = std::make_unique<hft::SpscQueue<std::uint64_t, 64>>();
        std::thread echo([&] {
            for (std::uint64_t i = 0; i < kTrips; ++i) {
                std::uint64_t v;
                while (!there->try_pop(v)) {
                }
                while (!back->try_push(v)) {
                }
            }
        });
        for (std::uint64_t i = 0; i < kTrips; ++i) {
            while (!there->try_push(i)) {
            }
            std::uint64_t v;
            while (!back->try_pop(v)) {
            }
        }
        echo.join();
    }, 3);
}
