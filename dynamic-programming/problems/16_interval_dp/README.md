# 16 · Interval DP: split points and "what happens last"

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★★☆ | 14 | interval DP with a split point (O(n³)), reconstructing a tree-shaped answer recursively, the "last operation" trick, adding sentinels |

## Why this matters

Problem 14 shrank intervals from the ends. The harder interval problems
**split** `[i, j]` at some `k` into two independent sub-intervals. This
pattern solves matrix chain ordering (the textbook example, and how query
optimizers order joins), optimal binary search trees, polygon triangulation
and many "merge/burst/cut" puzzles. The difficult part is choosing the state
so the two halves really are independent.

## The idea

### Matrix chain multiplication

Multiplying a `p×q` matrix by a `q×r` matrix costs `p·q·r` scalar
multiplications. For a chain `A₁A₂…Aₙ` with `Aᵢ` of size `dims[i-1] × dims[i]`,
the parenthesization changes the total cost enormously. Find the cheapest.

**State:** `C[i][j]` = cheapest cost to multiply `Aᵢ … Aⱼ` into one matrix.

**Recurrence:** the **last** multiplication done joins `(Aᵢ…Aₖ)` and
`(Aₖ₊₁…Aⱼ)` for some split `k`. Those two halves are independent problems:

```
C[i][i] = 0
C[i][j] = min over k in [i, j-1] of  C[i][k] + C[k+1][j] + dims[i-1]·dims[k]·dims[j]
```

Fill by increasing length `j - i` (problem 14). Three nested loops: O(n³).

To print the parenthesization, store the best `k` for each `(i, j)` in
`split[i][j]`, then recurse: `print(i, j) = "(" + print(i, k) + print(k+1, j) + ")"`,
with a single matrix printing its letter. The answer is a binary tree and
you rebuild it from the root.

### Burst balloons: think about the last one, not the first

Balloons `nums[0..n-1]`. Bursting balloon `i` earns
`left · nums[i] · right`, where `left`/`right` are its current neighbours
(1 if none). After it bursts, its neighbours become adjacent. Maximize the total.

If you pick the **first** balloon to burst, the remaining two sides are
*not* independent: their neighbours change. Instead pick the **last** balloon
`k` to burst inside an open interval `(i, j)`. While everything in `(i, k)`
and `(k, j)` is being burst, `k` is still there, acting as a wall, so the two
sides are independent, and when `k` finally bursts its neighbours are
exactly `i` and `j`:

```
pad nums with 1 at both ends: a = [1, nums..., 1]
B[i][j] = max coins from bursting everything strictly between i and j
B[i][j] = max over k in (i, j) of  B[i][k] + B[k][j] + a[i]·a[k]·a[j]
answer  = B[0][n+1]
```

"Choose what happens **last**" turns a messy problem into a clean split.
Remember it; it's the key to many hard interval problems.

### Cutting a stick

A stick of length `L` must be cut at given positions; each cut costs the
current length of the piece being cut. Order the cuts to minimize cost.
Sort the cuts, add `0` and `L` as sentinels, and it's the same structure:
`S[i][j] = (pos[j] - pos[i]) + min over k in (i, j) of S[i][k] + S[k][j]`.
Here the **first** cut in a piece is the one that splits it, and that works
because the cost doesn't depend on what happens inside the halves. Compare
with balloons and make sure you can say why one needs "first" and the other "last".

## Task

Implement in `interval_dp.hpp`:

| Function | Returns |
|---|---|
| `matrix_chain_cost(dims)` | min scalar multiplications. `dims.size() = n+1`, `1 <= n <= 300`, `dims[i] <= 100`. |
| `matrix_chain_order(dims)` | an optimal parenthesization string, matrices named `A`, `B`, `C`, …; `n <= 26`. Every product gets its own parentheses: `"((AB)C)"`. One matrix is just `"A"`. |
| `burst_balloons(nums)` | max coins. `n <= 300`, `0 <= nums[i] <= 100`. |
| `min_cut_cost(length, cuts)` | min total cost. `cuts.size() <= 300`, distinct, strictly inside `(0, length)`, any order. |

## Requirements

- O(n³) time, O(n²) memory. Use `int64_t` for costs.

<details>
<summary>Hint: loop bounds for burst balloons</summary>

`len` = `j - i` goes from 2 to `n + 1`; for each, `i` from 0 to `n + 1 - len`,
`j = i + len`, and `k` strictly between. Intervals with `j = i + 1` contain
no balloons and are 0.
</details>

## Going further

- Knuth's optimization: for the stick and optimal-BST problems the best split
  is monotone (`split[i][j-1] <= split[i][j] <= split[i+1][j]`), which brings
  O(n³) down to O(n²). Read about it after problem 20.
- Optimal binary search tree (CLRS 14.5): the same shape with access frequencies.
- Remove boxes (LeetCode 546): interval DP that needs a third index.

## Reading

- CLRS chapter 14.2 (15.2 in older editions), "Matrix-chain multiplication".
- LeetCode 312 (burst balloons) and 1547 (cut a stick).
