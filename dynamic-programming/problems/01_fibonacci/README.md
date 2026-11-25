# 01 · Fibonacci: your first memo table

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★☆☆☆☆ | nothing | overlapping subproblems, memoization (top-down), tabulation (bottom-up), rolling variables |

## Why this matters

Every dynamic programming (DP) solution you will ever write is the same trick:
**a recursive definition that would recompute the same thing many times, plus
a table so each thing is computed once.** Fibonacci is the smallest problem
where you can see that trick clearly, so we do it four ways.

## The idea

The definition is already a recurrence:

```
F(0) = 0, F(1) = 1
F(n) = F(n-1) + F(n-2)
```

Translating it straight into code (`fib_naive` in the header) is correct but
painfully slow. Draw the call tree for `F(5)`:

```
                 F5
           /            \
         F4              F3
       /    \          /    \
     F3      F2      F2      F1
    /  \    /  \    /  \
   F2  F1  F1  F0  F1  F0
  /  \
 F1  F0
```

`F3` is computed twice, `F2` three times. The tree roughly doubles at each
level, so the number of calls grows like 1.6ⁿ. `F(50)` takes about 40 billion
calls. Yet there are only 51 *distinct* questions (`F(0)` … `F(50)`). That gap
between "number of calls" and "number of distinct subproblems" is called
**overlapping subproblems**, and it is the signal that DP will help.

**Fix 1: memoization (top-down).** Keep the recursion, but remember answers.
Before computing `F(k)`, look in a cache; after computing it, store it. Each
`F(k)` is now computed once: O(n) time, O(n) memory.

**Fix 2: tabulation (bottom-up).** Notice that `F(k)` only needs smaller
indices. So fill an array from small to large with a plain loop, no recursion:

```
table[0] = 0; table[1] = 1;
for k in 2..n: table[k] = table[k-1] + table[k-2]
```

**Fix 3: rolling variables.** Each step only reads the last two entries, so
you only need to keep two numbers. O(1) memory. This "only keep the last
few rows" trick reappears in almost every problem in this repo.

### The five questions (use them for every problem)

1. **State:** what does one table entry mean? Here: `dp[k]` = the k-th Fibonacci number.
2. **Recurrence:** how is an entry built from smaller ones? `dp[k] = dp[k-1] + dp[k-2]`.
3. **Base cases:** which entries are known without the recurrence? `dp[0]`, `dp[1]`.
4. **Order:** in what order can you fill the table so dependencies are ready? Increasing `k`.
5. **Answer:** which entry (or combination) is the answer? `dp[n]`.

Write these five lines down *before* coding, every time. It is the single
best habit for getting good at DP.

## Task

Implement the TODOs in `fibonacci.hpp` (namespace `dp`):

| Function | What it returns |
|---|---|
| `fib_memo(n)` | `F(n)` using recursion + a cache (top-down) |
| `fib_table(n)` | `F(n)` using a `std::vector` filled by a loop (bottom-up) |
| `fib_two_vars(n)` | `F(n)` using O(1) extra memory |
| `naive_call_count(n)` | how many times `fib_naive(n)` would call itself, counting the first call. `naive_call_count(0) == 1`, `naive_call_count(2) == 3`. Do **not** actually call `fib_naive`: this count obeys its own recurrence. |
| `tribonacci(n)` | `T(0)=0, T(1)=1, T(2)=1, T(n)=T(n-1)+T(n-2)+T(n-3)`, any method |

## Requirements

- `0 <= n <= 93` for the Fibonacci functions (`F(93)` is the largest that fits in `uint64_t`).
- `0 <= n <= 70` for `naive_call_count` and `tribonacci`.
- Each call must be fast (the tests call `n = 93` thousands of times).
- Never call `fib_naive` from your solutions.

## Running the tests

```bash
./run_tests.sh             # build with -O2, ASan and UBSan, run all tests
./run_tests.sh asan memo   # only tests whose name contains "memo"
```

<details>
<summary>Hint 1: memo storage</summary>

Use a `std::vector<std::uint64_t>` of size `n + 1`, plus a way to say "not
computed yet" (a parallel `std::vector<bool>`, or `std::optional`). A
recursive lambda in C++20 needs a little trick: pass the lambda to itself
(`auto go = [&](auto& self, int k) -> std::uint64_t { ... self(self, k-1) ... };`),
or write a helper function that takes the cache by reference.
</details>

<details>
<summary>Hint 2: counting calls</summary>

A call to `fib_naive(k)` for `k >= 2` is one call plus everything its two
children do. So `calls(k) = 1 + calls(k-1) + calls(k-2)`. Base cases?
</details>

## Going further

- Time `fib_naive(40)` against `fib_two_vars(40)` with `./run_tests.sh release`.
- Compute `F(n) mod 1'000'000'007` for `n = 10^18` using 2×2 matrix
  exponentiation. That is O(log n) and shows up in interviews.
- Prove `naive_call_count(n) == 2 * F(n+1) - 1`.

## Reading

- *Introduction to Algorithms* (CLRS), chapter 14 (15 in older editions), "Dynamic Programming", intro section.
- Erik Demaine, MIT 6.006 Fall 2011, Lecture 19 "Dynamic Programming I" (free on MIT OpenCourseWare / YouTube).
