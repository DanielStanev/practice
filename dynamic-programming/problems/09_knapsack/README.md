# 09 · 0/1 knapsack: items used at most once, and swapping what you index by

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★☆☆ | 04, 08 | 2D state (items × capacity), compressing to 1D with a *downward* loop, reconstruction from a 2D table, subset sum, choosing which quantity to index by |

## Why this matters

Knapsack is the "take or skip" decision of problem 04 with a budget. It
models capital allocation, choosing which orders fit within a risk limit,
packing messages into a packet, and is asked in every DP interview round in
some disguise ("can this array be split into two equal halves?").

## The idea

Items have `weight[i]` and `value[i]`. Pick a subset with total weight
`<= capacity`, maximizing total value. **Each item at most once.**

**State:** `best[i][w]` = max value using only the first `i` items with
total weight at most `w`.

**Recurrence:** look at item `i-1`. Skip it, or take it (if it fits):

```
best[i][w] = max( best[i-1][w],
                  best[i-1][w - weight[i-1]] + value[i-1]   if weight[i-1] <= w )
best[0][w] = 0
answer = best[n][capacity]
```

Note the `i-1` on the right of the "take" branch: after taking item `i-1`
we look at the table *without* it. That is what "at most once" means.
In problem 08 the take branch looked at the same row, which is what made coins
reusable.

### From 2D to 1D: loop downwards

Row `i` only reads row `i-1`, so one array of size `capacity + 1` is enough,
updated in place. But order matters. If you loop `w` **upward**, then by the
time you compute `best[w]`, the cell `best[w - weight]` has already been
updated for item `i`, so you'd be allowed to take item `i` twice. That's
problem 08's unlimited coins. Loop `w` **downward** (`capacity … weight`) and
`best[w - weight]` still holds the old row:

```
for each item (wt, val):
    for w = capacity down to wt:
        best[w] = max(best[w], best[w - wt] + val)
```

This one-character difference (direction of the inner loop) between 0/1
and unbounded knapsack is a very common interview follow-up.

### Recovering the chosen items

For `knapsack_items` keep the full 2D table. Walk back from
`(n, capacity)`: if `best[i][w] != best[i-1][w]`, item `i-1` was taken, so
record it and move to `w - weight[i-1]`. Then `i--` either way.

### Subset sum

"Can `nums` be split into two groups with equal sums?" Both halves sum to
`total / 2`, so the question is: is there a subset summing to exactly
`total / 2`? That's knapsack with booleans: `can[s] = can[s] || can[s - x]`,
same downward loop. A `std::bitset` makes it a one-liner (`bits |= bits << x`)
and about 64× faster. Try both.

### When capacity is huge: index by the other quantity

If weights and capacity are up to 10^9, a table indexed by weight is
impossible. But if **values** are small (total value at most 10^5), flip it:

```
min_weight[v] = smallest total weight that achieves value exactly v
answer = largest v with min_weight[v] <= capacity
```

Same 0/1 recurrence, `min` instead of `max`, value as the index. The lesson:
**the table should be indexed by whichever quantity is small.** Read the
constraints first; they tell you the state.

## Task

Implement in `knapsack.hpp` (`Item{weight, value}`):

| Function | Returns |
|---|---|
| `knapsack(items, capacity)` | max total value. `n <= 200`, `capacity <= 100000`, weights `>= 1`. |
| `knapsack_items(items, capacity)` | indices (increasing) of an optimal subset |
| `can_partition_equal(nums)` | whether `nums` splits into two equal-sum groups. `n <= 200`, `nums[i] <= 1000`. |
| `knapsack_huge_capacity(items, capacity)` | max total value with weights and capacity up to 10^9, `n <= 100`, `value[i] <= 1000`. |

## Requirements

- O(n · capacity) for the first two, O(n · Σvalues) for the last.

<details>
<summary>Hint: initialising min_weight</summary>

`min_weight[0] = 0` and everything else is "infinity" (unreachable). Use
`int64_t` and an infinity like `INT64_MAX / 4` so adding a weight cannot overflow.
</details>

## Going further

- Count the subsets that sum to exactly `k` (the `+` version of subset sum).
- Multiple-choice knapsack: items come in groups, pick at most one per group.
- Read about the meet-in-the-middle approach when `n <= 40` and everything is huge.

## Reading

- AtCoder Educational DP Contest D and E (Knapsack 1 and 2): <https://atcoder.jp/contests/dp>.
- "Knapsack problems" chapter of *Competitive Programmer's Handbook* (Laaksonen).
