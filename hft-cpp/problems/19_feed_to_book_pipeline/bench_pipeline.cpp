// ./run_tests.sh bench
// End-to-end throughput: bytes in on this thread, book updated on the other.
// Try Config{"AAPL", <cpu>} with an isolated core and compare.
#include <algorithm>
#include <cstdint>
#include <random>
#include <span>

#include "bench.hpp"
#include "itch_wire.hpp"
#include "pipeline.hpp"

int main() {
    constexpr int kMessages = 1'000'000;
    wire::Writer w;
    std::mt19937_64 rng(1);
    for (int i = 1; i <= kMessages; ++i) {
        if (i % 3 == 0) w.del(std::uint64_t(i), std::uint64_t(i - 1));
        else w.add_order(std::uint64_t(i), std::uint64_t(i), rng() % 2 ? 'B' : 'S', 100, "AAPL",
                         1000000 + std::uint32_t(rng() % 64) * 100);
    }
    bench::run("pipeline: parse -> SPSC -> book -> seqlock", kMessages, [&] {
        hft::pipeline::Pipeline p(hft::pipeline::Config{"AAPL"});
        p.start();
        constexpr std::size_t kChunk = 1400;  // about one UDP packet
        for (std::size_t off = 0; off < w.bytes.size(); off += kChunk) {
            std::size_t n = std::min(kChunk, w.bytes.size() - off);
            p.on_bytes(std::span<const std::byte>(w.bytes.data() + off, n));
        }
        p.stop();
        bench::do_not_optimize(p.top());
    }, 3);
}
