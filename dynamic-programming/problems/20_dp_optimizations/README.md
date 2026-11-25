# 20 · DP optimizations: same recurrence, much faster

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★★★ | 03, 05, 10, 16 | sliding-window max with a monotonic deque, divide-and-conquer optimization (monotone split points), convex hull trick (min over lines) |

## Why this matters

Many DPs have the shape `dp[i] = min over j of (dp[j] + cost(j, i))`. Written
naively that's O(n²) (or O(k·n²) with layers). If `cost` has structure, you
can find the best `j` much faster **without changing the recurrence**. This
is exactly the HFT mindset: the maths is fixed, now make it fast. You saw the
first taste in problem 10 (LIS in O(n log n)); here are the three most
useful techniques.

Before each one: write the naive version first, make it pass the small
tests, then optimize and compare against it. That's how you'd do it in
an interview too.

## 1. Sliding window maximum: monotonic deque

`max_constrained_sum(nums, k)`: choose a non-empty subsequence where any two
consecutive chosen indices are at most `k` apart; maximize the sum.

```
best[i] = nums[i] + max(0, max(best[i-k], ..., best[i-1]))
answer  = max over i of best[i]
```

The inner max is over a **window** of the last `k` values: O(n·k) naively.
Keep a `std::deque` of indices whose `best` values are **decreasing**:

- before using it, pop from the front any index `< i - k` (left the window)
- the front is the window's max
- after computing `best[i]`, pop from the back every index with `best <= best[i]`
  (they are older *and* not better, so they can never be the max again), then push `i`

Each index is pushed and popped once: O(n) total.

## 2. Divide and conquer optimization

`min_partition_cost(a, k)`: split `a` (non-negative) into exactly `k`
non-empty contiguous groups, minimizing Σ (group sum)².

```
cost(j, i) = (prefix[i] - prefix[j])²                         group a[j..i-1]
dp[g][i]   = min over j < i of  dp[g-1][j] + cost(j, i)         first i elements in g groups
answer     = dp[k][n]
```

Naive: O(k · n²). With `n = 20000`, `k = 50` that's 2·10^10: hopeless.

Key fact: let `opt[g][i]` be the best `j` for `dp[g][i]`. For this cost
(and any cost satisfying the *quadrangle inequality*, which "square of a
sum of non-negatives" does), **`opt[g][i]` is non-decreasing in `i`**. So for
one layer `g`:

```
solve(lo, hi, opt_lo, opt_hi):        // compute dp[g][i] for i in [lo, hi],
    mid = (lo + hi) / 2               // knowing their opts are in [opt_lo, opt_hi]
    find best j for mid by scanning [opt_lo, min(mid-1, opt_hi)]
    solve(lo, mid-1, opt_lo, best_j)
    solve(mid+1, hi, best_j, opt_hi)
```

Every recursion level scans O(n) candidates in total and there are
O(log n) levels: O(n log n) per layer, O(k · n log n) overall.
Recursion depth is only log n here, so recursion is fine.

## 3. Convex hull trick

`frog_squared(h, C)`: stones with strictly increasing heights `h`. Jumping
from `j` to any `i > j` costs `(h[i] - h[j])² + C`. Minimum cost from stone 0
to the last stone.

```
dp[i] = min over j < i of  dp[j] + (h[i] - h[j])² + C
      = h[i]² + C + min over j of ( (-2·h[j]) · h[i] + (dp[j] + h[j]²) )
                                     slope m_j   x      intercept b_j
```

For each earlier `j` the bracket is a **line** `y = m_j·x + b_j`, evaluated at
`x = h[i]`. So `dp[i]` needs the minimum of a set of lines at one point.
Only lines on the **lower envelope** (the lower convex hull) can ever be
the minimum. Here slopes are added in decreasing order (`h` increases) and
query points increase, so:

- keep the envelope's lines in a deque
- **add** a line at the back: pop the back line while it's made useless
  (the new line and the one before it cover it everywhere)
- **query** at `x`: pop the front while the next line is at least as good at `x`
  (since `x` only grows, a popped line never comes back); answer with the front

O(n) total. Use `__int128` (or `long double` with care) for the "is this line
useless" comparison: it multiplies intercept differences by slope differences
and can exceed 64 bits. `__int128` is a GCC/Clang extension, so `-Wpedantic`
warns about it; declare `__extension__ using i128 = __int128;` once and use
`i128` to keep the build clean.

A line `(m2, b2)` between `(m1, b1)` and `(m3, b3)` (slopes decreasing) is
useless iff `(b3 - b1) · (m1 - m2) <= (b2 - b1) · (m1 - m3)`. Derive it yourself
from where the lines intersect. It's a classic interview derivation.

## Task

Implement in `dp_optimizations.hpp`:

| Function | Returns |
|---|---|
| `max_constrained_sum(nums, k)` | as described. `n <= 300000`, `1 <= k <= n`, `|nums[i]| <= 10^4`. |
| `min_partition_cost(a, k)` | as described. `n <= 20000`, `1 <= k <= min(n, 50)`, `0 <= a[i] <= 10^4`. |
| `frog_squared(h, C)` | as described. `n <= 300000`, `1 <= h[i] <= 10^6` strictly increasing, `1 <= C <= 10^12`. |

## Requirements

- O(n), O(k·n log n), O(n). The large tests time out for the naive versions.

<details>
<summary>Hint: the empty-group trap</summary>

In `min_partition_cost`, `dp[g][i]` for `i < g` is impossible (fewer elements
than groups). Use "infinity" there and make sure the D&C scan range for `j`
starts at `g - 1`.
</details>

## Going further

- Knuth's optimization (`opt[i][j-1] <= opt[i][j] <= opt[i+1][j]`) for interval DPs like problem 16's stick cutting: O(n³) → O(n²).
- Li Chao tree: the convex hull trick for lines and queries in any order, O(log n) each.
- "Aliens trick" (Lagrangian relaxation / WQS binary search): remove the `k` dimension from `min_partition_cost` entirely.

## Reading

- cp-algorithms.com: "Divide and Conquer DP", "Convex hull trick and Li Chao tree", "Minimum stack / Minimum queue".
- AtCoder Educational DP Contest Z ("Frog 3"): <https://atcoder.jp/contests/dp/tasks/dp_z>.
- After this repo, finish the whole AtCoder Educational DP Contest (26 problems). If you can do all of them, DP is no longer a weakness.
