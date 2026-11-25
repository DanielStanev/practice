// Tiny benchmarking helpers shared by the bench_*.cpp files.
// Build and run with:  ./run_tests.sh bench   (-O3 -march=native, no sanitizers)
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace bench {

// Prevents the compiler from optimising away a value or the work producing it.
template <typename T>
inline void do_not_optimize(T const& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

inline void clobber_memory() { asm volatile("" : : : "memory"); }

inline std::uint64_t now_ns() {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                          std::chrono::steady_clock::now().time_since_epoch())
                                          .count());
}

// Runs fn() `reps` times (after one warm-up run) and prints the best and
// median time per operation, where one run performs `ops` operations.
template <typename F>
void run(const char* name, std::uint64_t ops, F&& fn, int reps = 7) {
    fn();  // warm-up: page faults, branch predictors, caches
    std::vector<double> per_op;
    for (int r = 0; r < reps; ++r) {
        std::uint64_t t0 = now_ns();
        fn();
        std::uint64_t t1 = now_ns();
        per_op.push_back(static_cast<double>(t1 - t0) / static_cast<double>(ops));
    }
    std::sort(per_op.begin(), per_op.end());
    std::printf("%-44s best %8.2f ns/op   median %8.2f ns/op\n", name, per_op.front(),
                per_op[per_op.size() / 2]);
}

}  // namespace bench
