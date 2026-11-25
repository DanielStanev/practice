# 17 · Bitmask DP: when the state is a set

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★★☆ | 03, 09 | subsets as integers, bit tricks, O(2ⁿ·n²) Held-Karp TSP, using `popcount` to recover an implicit index, reconstruction through a set state |

## Why this matters

When `n` is small (≤ 20) and the answer depends on **which items have been
used**, not just how many, the state is a subset. Brute force over orders
is `n!` (20! ≈ 2.4·10^18); DP over subsets is `2ⁿ` ≈ 10^6. Bitmask DP also
forces you to be fluent with bit manipulation, which HFT interviews test
directly.

## The idea

### Sets as integers

Represent a subset of `{0, …, n-1}` as an integer `mask` whose bit `i` is set
iff `i` is in the set. Cheat sheet:

| Operation | Code |
|---|---|
| is `i` in the set? | `mask >> i & 1` |
| add `i` | `mask \| (1u << i)` |
| remove `i` | `mask & ~(1u << i)` |
| size of the set | `std::popcount(mask)` (C++20 `<bit>`) |
| all `n` items | `(1u << n) - 1` |
| lowest set bit | `mask & -mask`, index `std::countr_zero(mask)` |

A table indexed by `mask` has `2ⁿ` entries. For `n = 20` that's ~10^6.

### Travelling salesman (Held-Karp)

Visit every city exactly once starting and ending at city 0, minimizing total
distance (`dist[i][j]`, maybe asymmetric).

**State:** `best[mask][last]` = cheapest path that starts at 0, visits exactly
the cities in `mask` (which contains 0 and `last`), and ends at `last`.

The future only depends on *which* cities are done and *where you are*,
not on the order you visited them. That's why the order can be forgotten
and `n!` collapses to `2ⁿ·n`.

**Recurrence (push style):** from `(mask, last)` go to any unvisited `next`:

```
best[mask | 1<<next][next] = min(..., best[mask][last] + dist[last][next])
best[1][0] = 0
answer = min over last of best[all][last] + dist[last][0]
```

**Order:** every transition goes from `mask` to a strictly larger integer, so
plain increasing `mask` order is valid. O(2ⁿ · n²).

Reconstruction: store `parent[mask][last]`, then walk back from the best
`last` removing one city from the mask at each step.

### Assignment: let popcount be the index

Assign `n` workers to `n` jobs one-to-one, minimizing total cost. Process
workers in order 0, 1, 2…; then the state only needs **which jobs are
taken**: the number of taken jobs *is* the index of the next worker.

```
best[mask] = min cost to assign workers 0..popcount(mask)-1 to the jobs in mask
best[mask | 1<<j] = min(..., best[mask] + cost[popcount(mask)][j])     for j not in mask
```

O(2ⁿ · n) instead of O(2ⁿ · n²). Spotting quantities you don't need to store
because they follow from the rest of the state is a key optimization skill.

### Partition into k equal subsets

Can `nums` be split into `k` groups with equal sums? Fill the groups one at a
time. `fill[mask]` = how full the current group is after using the items in
`mask` (or -1 if unreachable). That's enough, because full groups are all
identical (sum `target`): `fill[mask] = sum(mask) % target`, and adding item
`i` is allowed only if it doesn't overflow the current group.

## Task

Implement in `bitmask_dp.hpp`:

| Function | Returns |
|---|---|
| `tsp_cost(dist)` | min cost of a tour from city 0 through all cities back to 0. `1 <= n <= 16`. |
| `tsp_tour(dist)` | one optimal tour as city indices, starting and ending with 0 (`n + 1` entries; `{0, 0}` for `n = 1`). |
| `min_assignment_cost(cost)` | min total cost of a perfect worker → job assignment. `n <= 20`. |
| `can_partition_k(nums, k)` | whether `nums` splits into `k` non-empty groups of equal sum. `n <= 16`, `1 <= k <= n`. |

Distances and costs are in `[0, 10^6]`.

## Requirements

- O(2ⁿ·n²), O(2ⁿ·n²), O(2ⁿ·n), O(2ⁿ·n).
- The tests use `n = 16` / `n = 20`, so n! or naive recursion won't finish.

<details>
<summary>Hint: memory layout for TSP</summary>

`std::vector<int64_t> best((1 << n) * n)`, index `mask * n + last`. With
`n = 16` that's about 8 MB. A `vector<vector<…>>` of 65536 small vectors is
much slower to allocate and to walk.
</details>

## Going further

- Count Hamiltonian paths in a graph with the same table (sum instead of min).
- Iterate over all **submasks** of a mask with `for (s = m; s; s = (s - 1) & m)`;
  total work over all masks is O(3ⁿ). Use it for "split the set into groups" problems.
- Profile DP (broken profile): tiling a grid row by row, where the mask is the jagged edge.

## Reading

- cp-algorithms.com: "Enumerating submasks of a bitmask" and "Bit manipulation".
- Henry S. Warren, *Hacker's Delight*: the bible of bit tricks.
