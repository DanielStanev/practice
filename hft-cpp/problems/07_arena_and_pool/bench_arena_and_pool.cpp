// ./run_tests.sh bench
#include <cstdint>
#include <random>
#include <vector>

#include "arena_and_pool.hpp"
#include "bench.hpp"

struct Order {
    std::uint64_t id;
    std::int64_t price;
    std::uint32_t qty;
    char symbol[8];
};

int main() {
    constexpr int kLive = 4096;
    constexpr int kOps = 1'000'000;

    // Same pseudo-random acquire/release pattern for both allocators.
    std::vector<std::uint32_t> pattern(kOps);
    std::mt19937 rng(1);
    for (auto& x : pattern) x = rng() % kLive;

    {
        std::vector<Order*> slots(kLive, nullptr);
        bench::run("new/delete (random churn)", kOps, [&] {
            for (int i = 0; i < kOps; ++i) {
                auto& s = slots[pattern[i]];
                if (s) {
                    delete s;
                    s = nullptr;
                } else {
                    s = new Order{std::uint64_t(i), i, 1, {}};
                }
                bench::do_not_optimize(s);
            }
        });
        for (auto* s : slots) delete s;
    }
    {
        hft::ObjectPool<Order> pool(kLive);
        std::vector<Order*> slots(kLive, nullptr);
        bench::run("ObjectPool (random churn)", kOps, [&] {
            for (int i = 0; i < kOps; ++i) {
                auto& s = slots[pattern[i]];
                if (s) {
                    pool.release(s);
                    s = nullptr;
                } else {
                    s = pool.acquire(Order{std::uint64_t(i), i, 1, {}});
                }
                bench::do_not_optimize(s);
            }
        });
        for (auto* s : slots)
            if (s) pool.release(s);
    }
    {
        hft::Arena arena(sizeof(Order) * kOps + 64);
        bench::run("Arena create (reset per run)", kOps, [&] {
            arena.reset();
            for (int i = 0; i < kOps; ++i) {
                Order* o = arena.create<Order>(Order{std::uint64_t(i), i, 1, {}});
                bench::do_not_optimize(o);
            }
        });
    }
}
