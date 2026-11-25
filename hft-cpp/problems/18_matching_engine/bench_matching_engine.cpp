// ./run_tests.sh bench
#include <cstdint>
#include <random>
#include <vector>

#include "bench.hpp"
#include "matching_engine.hpp"

using namespace hft::match;

struct Op {
    bool cancel;
    std::uint64_t id;
    Side side;
    std::int64_t price;
    std::uint32_t qty;
};

int main() {
    constexpr int kOps = 500'000;
    std::mt19937_64 rng(5);
    std::geometric_distribution<int> away(0.25);
    std::vector<Op> ops;
    std::vector<std::uint64_t> ids;
    for (int i = 0; i < kOps; ++i) {
        if (!ids.empty() && rng() % 100 < 40) {
            std::size_t k = rng() % ids.size();
            ops.push_back({true, ids[k], Side::Buy, 0, 0});
            ids[k] = ids.back();
            ids.pop_back();
        } else {
            Side s = rng() % 2 ? Side::Buy : Side::Sell;
            // Mostly passive, sometimes 1-2 ticks aggressive.
            std::int64_t px = s == Side::Buy ? 10000 - away(rng) + (rng() % 10 == 0 ? 2 : 0)
                                             : 10001 + away(rng) - (rng() % 10 == 0 ? 2 : 0);
            std::uint64_t id = std::uint64_t(i + 1);
            ops.push_back({false, id, s, px, 1 + std::uint32_t(rng() % 100)});
            ids.push_back(id);
        }
    }
    bench::run("matching engine mixed submit/cancel", kOps, [&] {
        MatchingEngine e(kOps);
        for (const auto& op : ops) {
            if (op.cancel) e.cancel(op.id);
            else e.submit(op.id, op.side, op.price, op.qty);
            if (e.trades().size() > 4096) e.clear_trades();
        }
        bench::do_not_optimize(e.resting_orders());
    }, 5);
}
