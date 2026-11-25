// ./run_tests.sh bench
// A realistic message mix: most activity near the top of the book, lots of
// adds and cancels, few executions.
#include <cstdint>
#include <random>
#include <vector>

#include "bench.hpp"
#include "order_book.hpp"

using hft::book::OrderBook;
using hft::book::Side;

struct Op {
    int kind;  // 0 add, 1 cancel(remove), 2 execute, 3 best-price query
    std::uint64_t ref;
    Side side;
    std::int64_t price;
    std::uint32_t qty;
};

int main() {
    constexpr int kOps = 1'000'000;
    std::mt19937_64 rng(77);
    std::geometric_distribution<int> distance(0.3);  // how far from the touch
    std::vector<Op> ops;
    std::vector<std::uint64_t> live;
    std::uint64_t next = 1;
    for (int i = 0; i < kOps; ++i) {
        int r = int(rng() % 100);
        if (r < 45 || live.size() < 100) {
            Side s = rng() % 2 ? Side::Buy : Side::Sell;
            std::int64_t px = s == Side::Buy ? 10000 - distance(rng) : 10001 + distance(rng);
            ops.push_back({0, next, s, px, 100});
            live.push_back(next++);
        } else if (r < 85) {
            std::size_t k = rng() % live.size();
            ops.push_back({1, live[k], Side::Buy, 0, 0});
            live[k] = live.back();
            live.pop_back();
        } else if (r < 90) {
            ops.push_back({2, live[rng() % live.size()], Side::Buy, 0, 10});
        } else {
            ops.push_back({3, 0, Side::Buy, 0, 0});
        }
    }
    bench::run("order book mixed ops", kOps, [&] {
        OrderBook b;
        std::int64_t sink = 0;
        for (const auto& op : ops) {
            switch (op.kind) {
                case 0: b.add(op.ref, op.side, op.price, op.qty); break;
                case 1: b.remove(op.ref); break;
                case 2: b.execute(op.ref, op.qty); break;
                case 3:
                    if (auto l = b.best_bid()) sink += l->price;
                    break;
            }
        }
        bench::do_not_optimize(sink);
    }, 5);
}
