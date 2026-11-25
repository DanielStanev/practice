// ./run_tests.sh bench
// Compares scans over AoS and SoA layouts. Try different store sizes: once
// the data no longer fits in L2/L3 the gap widens.
#include <cstdint>
#include <cstdio>
#include <random>

#include "bench.hpp"
#include "order_store.hpp"

template <typename Store>
void run_suite(const char* label, std::size_t n) {
    Store s(n);
    std::mt19937_64 rng(1);
    for (std::size_t i = 0; i < n; ++i)
        s.add(i, rng() % 2 ? hft::Side::Buy : hft::Side::Sell, 10000 + std::int64_t(rng() % 200),
              1 + std::uint32_t(rng() % 100), i);
    char name[96];
    std::snprintf(name, sizeof name, "%s qty_at_or_better (n=%zu)", label, n);
    bench::run(name, n, [&] { bench::do_not_optimize(s.qty_at_or_better(hft::Side::Buy, 10100)); });
    std::snprintf(name, sizeof name, "%s total_qty (n=%zu)", label, n);
    bench::run(name, n, [&] { bench::do_not_optimize(s.total_qty(hft::Side::Sell)); });
    std::snprintf(name, sizeof name, "%s count_older_than (n=%zu)", label, n);
    bench::run(name, n, [&] { bench::do_not_optimize(s.count_older_than(n / 2)); });
}

int main() {
    for (std::size_t n : {10'000u, 1'000'000u}) {
        run_suite<hft::AosOrderStore>("AoS", n);
        run_suite<hft::SoaOrderStore>("SoA", n);
    }
}
