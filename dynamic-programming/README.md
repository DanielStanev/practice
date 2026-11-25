# Dynamic Programming, From Zero

Twenty C++20 problems that take you from "what is memoization?" to the DP
optimizations that show up in the hardest interview rounds. It starts
**very** gently (Fibonacci, climbing stairs) and adds one idea at a time.
Every README teaches the idea it introduces before asking you to use it, with
a worked example, so you can learn DP from this repo alone.

The setup matches the HFT problems repo: a spec, a header with `TODO`
stubs, a test suite, and `-O2` with AddressSanitizer and
UndefinedBehaviorSanitizer on by default. Most tests compare your answer with
a brute-force solution on thousands of small random inputs, then run large
inputs that only a correct-complexity solution finishes in time.

## Quick start

Requirements: Linux, macOS or WSL2, GCC 12+ or Clang 16+, Bash. CMake is optional.

```bash
cd problems/01_fibonacci
cat README.md            # the lesson and the task
$EDITOR fibonacci.hpp    # implement the TODOs
./run_tests.sh           # build with -O2 + ASan + UBSan and run the tests
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
| `./run_tests.sh` | `-O2` + ASan + UBSan |
| `./run_tests.sh asan memo` | same, only tests whose name contains `memo` |
| `./run_tests.sh debug` | `-O0` + sanitizers, best for stepping through in gdb/lldb |
| `./run_tests.sh release` | `-O3 -march=native`, no sanitizers, to feel real speed |

`CXX=clang++ ./run_tests.sh` switches compilers. `TIMEOUT=300 ./run_tests.sh`
gives slow machines more time (default 120 s). A timeout almost always means
your solution has the wrong complexity, not that the machine is slow.

All tests fail until you implement the header. That's expected: the stubs
compile and throw `std::logic_error("TODO: ...")`.

## The recipe (read this first)

Every DP solution in this repo comes from answering five questions. Write the
answers down, in words, before writing code:

1. **State:** what does one table entry mean, exactly? ("`best[i]` = the most money from the first `i` houses")
2. **Recurrence:** how is an entry built from smaller ones? Usually by asking "what was the **last** decision?"
3. **Base cases:** which entries do you know directly? (Empty prefix, zero items, one character…)
4. **Order:** in what order can you fill the table so every dependency is ready?
5. **Answer:** which entry, or combination of entries, is the answer?

Then: write the slow version first, check it against brute force, and only
then optimize memory or time.

## The problems

| # | Problem | Difficulty | Builds on | You learn |
|---|---|---|---|---|
| 01 | [Fibonacci](problems/01_fibonacci) | ★☆☆☆☆ | | overlapping subproblems, memoization, tabulation, O(1) memory |
| 02 | [Climbing stairs](problems/02_climbing_stairs) | ★☆☆☆☆ | 01 | finding a recurrence from the last move, counting mod 1e9+7, adding info to the state |
| 03 | [Min-cost stairs and frog](problems/03_min_cost_stairs) | ★☆☆☆☆ | 02 | `min` instead of `+`, infinity sentinels, reconstructing the path |
| 04 | [House robber](problems/04_house_robber) | ★☆☆☆☆ | 02, 03 | take/skip, prefix states, splitting into cases, reductions |
| 05 | [Maximum subarray](problems/05_max_subarray) | ★★☆☆☆ | 03, 04 | "best ending here" (Kadane), one-trade stock profit, two-value states |
| 06 | [Decode ways and word break](problems/06_decode_ways) | ★★☆☆☆ | 02, 04 | DP over string prefixes, count / bool / min on the same table |
| 07 | [Grid paths](problems/07_grid_paths) | ★★☆☆☆ | 02, 03 | 2D tables, fill order, one-row memory, flat arrays |
| 08 | [Coin change](problems/08_coin_change) | ★★☆☆☆ | 02, 03 | unbounded knapsack, combinations vs permutations (loop order) |
| 09 | [0/1 knapsack](problems/09_knapsack) | ★★★☆☆ | 04, 08 | items × capacity, downward 1D loop, subset sum, indexing by the small quantity |
| 10 | [Longest increasing subsequence](problems/10_longest_increasing_subsequence) | ★★★☆☆ | 03, 05 | O(n²) → O(n log n), reconstruction, sorting reductions |
| 11 | [Longest common subsequence](problems/11_longest_common_subsequence) | ★★★☆☆ | 05, 07, 10 | DP over two strings, table walk-back, substring vs subsequence |
| 12 | [Edit distance](problems/12_edit_distance) | ★★★☆☆ | 11 | non-zero base cases, producing an edit script, weighted costs |
| 13 | [Stock trading state machines](problems/13_stock_trading) | ★★★☆☆ | 02, 04, 05 | DP as states + transitions, fees, cooldown, k trades |
| 14 | [Palindromes](problems/14_palindromes) | ★★★☆☆ | 07, 11 | interval DP, filling by length, interval table + prefix DP |
| 15 | [Probability and expected value](problems/15_probability_dp) | ★★★☆☆ | 02, 08, 13 | probability DP, optimal stopping, self-loops, runs of heads |
| 16 | [Interval DP with split points](problems/16_interval_dp) | ★★★★☆ | 14 | matrix chain, burst balloons ("what happens last"), O(n³) |
| 17 | [Bitmask DP](problems/17_bitmask_dp) | ★★★★☆ | 03, 09 | sets as integers, Held-Karp TSP, assignment, popcount as an index |
| 18 | [Tree DP](problems/18_tree_dp) | ★★★★☆ | 04, 05 | subtree states, diameter, rerooting, iterative DFS for deep trees |
| 19 | [Digit DP](problems/19_digit_dp) | ★★★★☆ | 02, 15 | counting up to 10^18, tight / started flags, `F(hi) - F(lo-1)` |
| 20 | [DP optimizations](problems/20_dp_optimizations) | ★★★★★ | 03, 05, 10, 16 | monotonic deque, divide & conquer optimization, convex hull trick |

Problem 15 is the one most directly asked at trading firms (dice games,
optimal stopping, expected values). Don't skip it.

## How to work through it

1. Read the problem's README slowly. Do the worked example by hand on paper.
2. Write the five answers from "The recipe" above, then code the simplest correct version.
3. Run the tests. When a random brute-force comparison fails, the message shows the line and both values but not the random input; print the input from the test, or reproduce it with `./run_tests.sh debug <test name>` under a debugger.
4. When it passes, do the memory/time improvements the README suggests.
5. Do at least one "Going further" item. That's where the idea sticks.
6. Come back a week later and redo the problem from a blank header without looking. Spaced repetition matters more for DP than for anything else.

If you get stuck, each README has collapsed hints. Try for at least 30
minutes before opening them. Reference solutions live on the `solutions`
branch (see the top-level README); use them to
compare *after* you pass, not before.

## Layout

```
common/
  testing.hpp        minimal test harness (TEST, CHECK, CHECK_EQ, CHECK_NEAR, ...)
  run_problem.sh     build/run logic shared by every run_tests.sh
problems/NN_name/
  README.md          the lesson, the task, hints, further reading
  <name>.hpp         the file you edit
  test_<name>.cpp    the tests (brute-force cross-checks + large inputs)
  run_tests.sh       build and run
run_all.sh           scoreboard
CMakeLists.txt       optional CMake/CTest build
```

## After this repo

- AtCoder Educational DP Contest (<https://atcoder.jp/contests/dp>): 26 classic problems; several of them are in here, the rest are the natural next step.
- CSES Problem Set, "Dynamic Programming" section (<https://cses.fi/problemset/>).
- *Competitive Programmer's Handbook* (Antti Laaksonen, free PDF), chapters 7 and 10.
