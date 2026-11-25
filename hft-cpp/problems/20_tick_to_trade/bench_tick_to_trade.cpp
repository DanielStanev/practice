// ./run_tests.sh bench
// Prints the tick-to-trade latency distribution for a synthetic burst
// pattern. Look at p50 vs p99 vs max, then try to shrink the tail.
#include <cstdint>
#include <cstdio>
#include <span>

#include "itch_wire.hpp"
#include "tick_to_trade.hpp"

using namespace hft::t2t;

int main() {
    RiskLimits limits{1000, 1'000'000'000, 1'000'000, 1'000'000, 1'000'000};
    TickToTrade t(Config{"AAPL", StrategyParams{0.6, 10}, limits});
    std::uint64_t ref = 1;
    std::uint64_t ts = 1;
    for (int i = 0; i < 200'000; ++i) {
        wire::Writer w;
        std::uint64_t a = ref++, b = ref++;
        w.add_order(ts, a, 'S', 100, "AAPL", 1000100);
        w.add_order(ts, b, 'B', 900, "AAPL", 1000000);  // fires a buy
        w.del(ts, b);
        w.del(ts, a);
        t.on_bytes(std::span<const std::byte>(w.bytes.data(), w.bytes.size()), ts);
        ts += 1000;
        if (t.wire_out().size() > (1 << 20)) t.clear_wire_out();
    }
    std::printf("orders sent: %zu\n", t.sent().size());
    std::printf("tick-to-trade ns: %s\n", t.latency().summary().c_str());
}
