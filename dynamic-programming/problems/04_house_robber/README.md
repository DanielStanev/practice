# 04 · House robber: take it or skip it

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★☆☆☆☆ | 02, 03 | the take/skip decision, prefix states, splitting a hard case into easy ones, reducing a new problem to a known one |

## Why this matters

"Do I take item `i` or not?" is the decision behind knapsack (problem 09),
scheduling, portfolio selection and half of all interview DP. Here it appears
in its simplest form.

## The idea

Houses in a row hold `money[0..n-1]`. You may not take from two adjacent
houses. Maximize the total.

**State:** `best[i]` = the most you can get using only houses `0 … i-1`
(the first `i` houses). This "answer for a prefix" state is the most common
1D state there is.

**Recurrence:** look at the last house in the prefix, house `i-1`. Either

- you **skip** it: then the best is `best[i-1]`, or
- you **take** it: then house `i-2` is off limits, so you get `money[i-1] + best[i-2]`.

```
best[i] = max(best[i-1], money[i-1] + best[i-2])
best[0] = 0, best[1] = money[0]
```

Example, `money = [2, 7, 9, 3, 1]`:

```
i       : 0  1  2  3   4   5
best[i] : 0  2  7  11  11  12     (take 2, 9, 1)
```

### Circular street

If the houses form a circle, house 0 and house `n-1` are neighbours. The
circle breaks the simple "prefix" structure, but you can split it into two
cases that are both lines:

- house 0 is not taken → solve the line `money[1 … n-1]`
- house `n-1` is not taken → solve the line `money[0 … n-2]`

Every valid plan is in at least one case, so the answer is the max of the
two. **Splitting into cases that each look like an easier problem** is a
technique you will use constantly. Watch the `n = 1` edge case.

### Reductions

`delete_and_earn`: you pick a value `x` from `nums`, earn `x` for **each**
copy of `x`, but then you may not pick `x - 1` or `x + 1` at all. That sounds
new, but bucket the values: `gain[x] = x * count(x)`. Picking `x` forbids its
neighbours `x ± 1`… which is exactly house robber over `gain[0 … max]`.
Recognising "this is a problem I already know" is the most valuable DP skill
of all, so look for it in every later problem.

## Task

Implement in `house_robber.hpp`:

| Function | Returns |
|---|---|
| `rob_line(money)` | max total with no two adjacent houses. Empty input → 0. |
| `rob_circle(money)` | same, but the first and last houses are adjacent. |
| `delete_and_earn(nums)` | max points as described. `nums[i]` in `[1, 100000]`. |

All sizes up to 200000, values up to 10^9 (`rob_*`). Use `int64_t`.

## Requirements

- O(n) time (O(n + max value) for `delete_and_earn`), O(1) extra memory for `rob_line`.

<details>
<summary>Hint: O(1) memory</summary>

`best[i]` only reads `best[i-1]` and `best[i-2]`. Keep two variables, like
`fib_two_vars` in problem 01.
</details>

<details>
<summary>Hint: rob_circle without copying</summary>

Write a helper `rob_range(money, first, last)` that solves the line on a
sub-range, and call it twice.
</details>

## Going further

- Return which houses to rob (parent pointers, like problem 03).
- Houses on a binary tree, where you cannot rob a parent and child together.
  You will do this in problem 18.

## Reading

- LeetCode 198, 213 and 740 are this problem's three parts.
