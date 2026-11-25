# C++ Practice

Three sets of C++20 practice problems, built like a timed assessment: each
problem has a README that teaches the idea and gives the spec, a header with
`TODO` stubs for you to fill in, a test suite, and sanitizers on by default.

| Set | Folder | Problems | What it drills |
|-----|--------|---------:|----------------|
| HFT C++ | [`hft-cpp/`](hft-cpp/) | 20 | RAII and smart pointers up to lock-free queues, an ITCH parser, an order book, a matching engine and a tick-to-trade path |
| Dynamic programming | [`dynamic-programming/`](dynamic-programming/) | 20 | From Fibonacci and memoization to bitmask, tree and digit DP and DP optimizations (deque, divide and conquer, convex hull trick) |
| C++ idioms and iterators | [`cpp-idioms/`](cpp-idioms/) | 42 | Iterators, STL algorithms and ranges, value categories and moves, forwarding, RAII, rule of 0/3/5, SFINAE/concepts, CRTP, constexpr, lambdas, lifetime and UB |

Each set stands on its own: its own README with the full problem table,
`common/` test harness, `run_all.sh` scoreboard and optional `CMakeLists.txt`.

## Suggested order

The sets are meant to be worked in parallel rather than one after another:

- **cpp-idioms** groups A–J (problems 01–30) make **hft-cpp** 01–10 much
  easier, so start there or alternate between the two.
- **hft-cpp** 11 onward is the systems half: concurrency, lock-free queues,
  networking and the order book. Problems 17, 19 and 20 `#include` your
  earlier solutions.
- **dynamic-programming** is the interview-algorithms track. A problem or two
  a week alongside the others works well; it starts very gently.

## Quick start

Requirements: Linux, macOS (DP set) or WSL2, GCC 12+ or Clang 16+, Bash.
CMake is optional.

```bash
cd hft-cpp/problems/01_unique_ptr
cat README.md          # the task
$EDITOR unique_ptr.hpp # implement the TODOs
./run_tests.sh         # build with -O2 + ASan + UBSan and run the tests
```

From the repository root:

```bash
./run_all.sh                  # scoreboard for every set
./run_all.sh dp               # one set: hft | dp | idioms
./run_all.sh hft -- 05 12     # options after -- go to the set's run_all.sh
./run_all.sh --compile-only   # check every problem compiles
```

Or with CMake and CTest:

```bash
cmake -S . -B build && cmake --build build -j
ctest --test-dir build --output-on-failure -R 09_knapsack
```

Every problem fails out of the box: the stubs throw `std::logic_error("TODO")`.

## Layout

```
README.md            this file
run_all.sh           scoreboard across sets
CMakeLists.txt       optional CMake build of all sets
hft-cpp/             20 HFT systems problems
dynamic-programming/ 20 DP problems
cpp-idioms/          42 language-idiom problems
  <set>/README.md          set overview and problem table
  <set>/common/            testing.hpp and shared build script
  <set>/problems/NN_name/  README.md, <name>.hpp (you edit), test_<name>.cpp, run_tests.sh
```

## Reference solutions

Solutions are kept off `main` on the **`solutions`** branch, so you never see
one by accident while working. That branch is `main` plus a `solutions/`
folder (`solutions/<set>/NN_name/<name>.hpp`). Look at one only after your
tests pass:

```bash
git fetch origin solutions
git show origin/solutions:solutions/dynamic-programming/09_knapsack/knapsack.hpp
```

Keep your own work on `main` (or your own branches) and never merge
`solutions` into it.
