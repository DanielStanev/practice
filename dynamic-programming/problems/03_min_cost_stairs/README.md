# 03 · Min-cost stairs: optimizing instead of counting, and recovering the path

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★☆☆☆☆ | 02 | optimization DP (`min` instead of `+`), "infinity" sentinels, reconstructing the choices with parent pointers, transitions over a window |

## Why this matters

Most interview and real-world DP asks for the **best** something (cheapest,
fastest, most profitable), and then usually asks **which choices** achieve
it. This problem teaches both on the simplest possible setting.

## The idea

`cost[i]` is the price of standing on step `i` (steps `0 … n-1`). You start
on step 0 or step 1 (your choice, free to pick), pay for every step you
stand on, move up 1 or 2 steps at a time, and finish when you pass the last
step (reach position `n`, "the top", which costs nothing).

The "last move" argument from problem 02 still works. To be at position `k`
you came from `k-1` or `k-2`. When counting we *added* the two options.
When optimizing we take the *better* one:

```
best[k] = cost[k] + min(best[k-1], best[k-2])      (cost[n] = 0 for the top)
best[0] = cost[0], best[1] = cost[1]
answer  = best[n]
```

Worked example, `cost = [10, 15, 20]`:

```
k        : 0   1   2   3(top)
best[k]  : 10  15  30  15      <- best[3] = 0 + min(best[2], best[1]) = 15
```

Start on step 1, pay 15, jump 2 to the top.

### Recovering the path

`best[]` tells you the cost, not the route. To get the route, store for every
`k` **which predecessor won the `min`** (`parent[k] = k-1` or `k-2`). Then
walk back from the top: `n → parent[n] → parent[parent[n]] → …` until you hit
a starting step, and reverse. This works for every optimization DP in this
repo; you will do it again for knapsack, LIS, LCS and TSP.

### Wider windows and "infinity"

In `frog_min_energy`, a frog on stone `i` can jump to any of `i+1 … i+k`, and
a jump from `i` to `j` costs `|h[i] - h[j]|`. Now `best[j]` looks at up to `k`
predecessors:

```
best[j] = min over i in [j-k, j-1], i >= 0 of  best[i] + |h[i] - h[j]|
```

When you take a `min` you need a starting value that loses to everything.
Use a large constant like `INT64_MAX / 4` (not `INT64_MAX` itself: adding to
it overflows, and signed overflow is undefined behaviour, which UBSan will
catch).

## Task

Implement in `min_cost_stairs.hpp`:

| Function | Returns |
|---|---|
| `min_cost_climb(cost)` | minimum total cost to reach the top, as described above. `2 <= cost.size() <= 200000`, `0 <= cost[i] <= 10^9`. |
| `min_cost_path(cost)` | the steps you stand on (indices, increasing) in one cheapest route. Any optimal route is accepted. |
| `frog_min_energy(h, k)` | minimum energy for the frog to go from stone 0 to stone `h.size()-1`. `1 <= h.size() <= 100000`, `1 <= k <= 100`, `1 <= h[i] <= 10^9`. |

## Requirements

- O(n) for the first two, O(n·k) for the frog.
- Use `int64_t` for sums: 200000 × 10^9 does not fit in `int`.

<details>
<summary>Hint: where does the path start?</summary>

Give the start steps no parent (for example `-1`). When walking back from
the top you stop when the parent is `-1`. The top itself (index `n`) is not a
step you stand on, so do not include it in the result.
</details>

## Going further

- Solve the frog with "push" style: from each stone `i`, update `best[i+1..i+k]`.
  Both styles are correct; get comfortable with both.
- Return the frog's path too.

## Reading

- AtCoder "Educational DP Contest", problems A and B (Frog 1 and Frog 2): <https://atcoder.jp/contests/dp>. The whole contest is excellent practice once you finish this repo.
