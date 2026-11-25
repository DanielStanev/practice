// ./run_tests.sh bench
// Messages per second through your parser with a trivial handler.
#include <cstdint>
#include <random>
#include <span>

#include "bench.hpp"
#include "itch_parser.hpp"
#include "itch_wire.hpp"

namespace itch = hft::itch;

struct Summer {
    std::uint64_t sum = 0;
    void on_add(const itch::AddOrder& m) { sum += m.price + m.shares; }
    void on_execute(const itch::OrderExecuted& m) { sum += m.executed_shares; }
    void on_cancel(const itch::OrderCancel& m) { sum += m.cancelled_shares; }
    void on_delete(const itch::OrderDelete& m) { sum += m.order_ref; }
    void on_replace(const itch::OrderReplace& m) { sum += m.price; }
};

int main() {
    constexpr int kMessages = 1'000'000;
    wire::Writer w;
    std::mt19937 rng(3);
    for (int i = 0; i < kMessages; ++i) {
        switch (rng() % 10) {
            case 0: case 1: case 2: case 3: w.add_order(i, i, 'B', 100, "AAPL", 1500000 + rng() % 100); break;
            case 4: case 5: w.del(i, i); break;
            case 6: case 7: w.cancel(i, i, 10); break;
            case 8: w.executed(i, i, 10, i); break;
            case 9: w.replace(i, i, i + 1, 50, 1500000); break;
        }
    }
    std::span<const std::byte> buf(w.bytes.data(), w.bytes.size());
    bench::run("parse mixed messages", kMessages, [&] {
        Summer s;
        auto r = itch::parse(buf, s);
        bench::do_not_optimize(r);
        bench::do_not_optimize(s.sum);
    });
}
