# HFT C++ Problems

Twenty C++20 problems that take you from RAII and move semantics to a
multi-threaded, measured tick-to-trade path: the skills trading firms test
for and use every day. Each problem teaches something new and later
problems reuse your earlier solutions, so by problem 20 you are wiring your
own parser, order book, queues and histogram into one system.

Every problem is set up like a timed assessment: a spec, a header with the
interface and `TODO` stubs, a test suite you have not seen the inside of yet
(you can read it, but try the spec first), sanitizers on by default, and a
benchmark where speed is part of the point.

## Quick start

Requirements: Linux (or WSL2), GCC 12+ or Clang 16+, Bash. CMake is optional.

```bash
cd problems/01_unique_ptr
cat README.md          # the task
$EDITOR unique_ptr.hpp # implement the TODOs
./run_tests.sh         # build with -O2 + ASan + UBSan and run the tests
```

From this folder (the set root):

```bash
./run_all.sh                 # scoreboard for all 20 problems
./run_all.sh 05 06           # just problems 05 and 06
./run_all.sh --compile-only  # check everything compiles
```

Each problem's `run_tests.sh` supports modes:

| Command | What it does |
|---|---|
| `./run_tests.sh` | default modes: ASan+UBSan, plus TSan for concurrent problems |
| `./run_tests.sh asan reset` | ASan+UBSan, only tests whose name contains `reset` |
| `./run_tests.sh tsan` | ThreadSanitizer |
| `./run_tests.sh release` | `-O3 -march=native`, no sanitizers |
| `./run_tests.sh bench` | build and run `bench_*.cpp` (problems that have one) |

`CXX=clang++ ./run_tests.sh` switches compilers. CMake users: see the top of
`CMakeLists.txt`.

All tests fail until you implement the header. That is expected: the stubs
compile and throw `std::logic_error("TODO: ...")` or return placeholder values.

## The problems

| # | Problem | Difficulty | Builds on | You learn |
|---|---|---|---|---|
| 01 | [UniquePtr](problems/01_unique_ptr) | ★☆☆☆☆ | | RAII, move semantics, rule of five, `noexcept`, `[[no_unique_address]]` |
| 02 | [Vector with strong exception guarantee](problems/02_vector) | ★☆☆☆☆ | 01 | raw storage vs. objects, `move_if_noexcept`, exception safety, aliasing |
| 03 | [SharedPtr / WeakPtr](problems/03_shared_ptr) | ★★☆☆☆ | 01, 02 | control blocks, atomic ref counts, `weak_ptr::lock` race |
| 04 | [Type traits and constexpr](problems/04_type_traits) | ★★☆☆☆ | 01–03 | partial specialisation, type lists, fold expressions, concepts |
| 05 | [Fixed-point Price](problems/05_fixed_point_price) | ★★☆☆☆ | 04 | strong value types, overflow checks, `<=>`, parsing without floats |
| 06 | [SmallFunction](problems/06_small_function) | ★★★☆☆ | 01–04 | type erasure, small-buffer optimisation, hand-made vtables |
| 07 | [Arena and object pool](problems/07_arena_and_pool) | ★★★☆☆ | 02, 03 | bump allocation, intrusive free lists, allocators, no heap on hot path |
| 08 | [Flat hash map](problems/08_flat_hash_map) | ★★★☆☆ | 02, 04, 07 | open addressing, backward-shift deletion, hash mixing |
| 09 | [Latency histogram](problems/09_latency_histogram) | ★★★☆☆ | 04, 07 | HDR histograms, percentiles, `rdtsc`, measuring tails |
| 10 | [AoS vs SoA order store](problems/10_soa_order_store) | ★★★☆☆ | 05, 08, 09 | data-oriented design, cache lines, branch-free loops, SIMD |
| 11 | [Spinlock and blocking queue](problems/11_spinlock_and_blocking_queue) | ★★★☆☆ | 03, 09 | acquire/release, TTAS, condition variables, shutdown |
| 12 | [Lock-free SPSC queue](problems/12_spsc_queue) | ★★★★☆ | 02, 04, 11 | lock-free ring buffer, false sharing, cached indices |
| 13 | [Seqlock](problems/13_seqlock) | ★★★★☆ | 11, 12 | optimistic reads, fences, why "benign" races are UB |
| 14 | [Bounded MPMC queue](problems/14_mpmc_queue) | ★★★★☆ | 07, 12 | CAS loops, per-slot sequence numbers, Vyukov's design |
| 15 | [ITCH parser](problems/15_itch_parser) | ★★★☆☆ | 04, 05 | binary protocols, endianness, strict aliasing, streaming |
| 16 | [UDP feed handler](problems/16_udp_feed_handler) | ★★★★☆ | 07, 15 | sockets, epoll, A/B arbitration, gap detection |
| 17 | [L2 order book](problems/17_l2_order_book) | ★★★★☆ | 05, 08, 10, 15 | book semantics, container choice, model-based testing |
| 18 | [Matching engine](problems/18_matching_engine) | ★★★★☆ | 07, 08, 17 | price-time priority, order types, intrusive queues |
| 19 | [Feed-to-book pipeline](problems/19_feed_to_book_pipeline) | ★★★★★ | 12, 13, 15, 17 | composing lock-free parts, thread lifecycle, CPU pinning |
| 20 | [Tick-to-trade capstone](problems/20_tick_to_trade) | ★★★★★ | 09, 15, 17 | risk checks, rate limits, end-to-end latency |

Problems 17, 19 and 20 `#include` your solutions to earlier problems, so
they only pass once those do.

## How to work through it

1. Read the problem's README and the header. Write down your design before coding.
2. Make the tests pass under ASan+UBSan (and TSan where it runs).
3. Run the benchmark if there is one. Understand the number.
4. Do at least one "Going further" item. That is where most of the learning is.
5. Read the linked material for the problem. Watch the talks.
6. Commit your solution on your own branch so you can compare later attempts.

If you get stuck, each README has collapsed hints. Try for at least an hour
before opening them. Reference solutions live on the `solutions` branch (see
the top-level README).

## Layout

```
common/
  testing.hpp        minimal test harness (TEST, CHECK, CHECK_EQ, ...)
  bench.hpp          benchmark helpers (do_not_optimize, bench::run)
  alloc_counter.hpp  counts operator new calls, for "must not allocate" tests
  itch_wire.hpp      independent reference encoder used by tests 15, 16, 19, 20
  run_problem.sh     build/run logic shared by every run_tests.sh
problems/NN_name/
  README.md          the task, requirements, hints, further reading
  <name>.hpp         the file you edit
  test_<name>.cpp    the tests
  bench_<name>.cpp   benchmark (some problems)
  run_tests.sh       build and run
run_all.sh           scoreboard
CMakeLists.txt       optional CMake/CTest build
```

Benchmark numbers in the READMEs were measured on a shared cloud VM. Expect
your own machine to differ; what matters is the comparison between variants.
