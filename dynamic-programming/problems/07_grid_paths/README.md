# 07 · Grid paths: your first 2D table

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★☆☆☆ | 02, 03 | 2D state `dp[r][c]`, row-major fill order, boundary handling, keeping one row (O(cols) memory), flat arrays for speed |

## Why this matters

Many DP problems have a state with two indices: a position in a grid, or a
position in each of two strings (LCS, edit distance, problems 11-12). Grids
are the easiest place to get comfortable with 2D tables, their fill order and
their edges.

## The idea

A robot starts in the top-left cell of an `R × C` grid and moves only
**right** or **down**. How many paths reach the bottom-right cell?

**State:** `paths[r][c]` = number of paths from `(0,0)` to `(r,c)`.
**Last move:** you arrived from above `(r-1, c)` or from the left `(r, c-1)`:

```
paths[r][c] = paths[r-1][c] + paths[r][c-1]      (missing neighbours count as 0)
paths[0][0] = 1
```

**Order:** each cell needs the cell above and the cell to the left, so fill
row by row, left to right. Every 2D problem forces you to answer "what does
this cell depend on, and does my loop order compute those first?"

```
1  1  1  1
1  2  3  4
1  3  6 10      3 × 4 grid → 10 paths
```

**Obstacles.** A blocked cell has `paths = 0`. That's the only change, and
it is also why the starting cell needs care: if it's blocked, the answer is 0.

**Min path sum.** Same shape, `min` instead of `+`, and add the cell's own cost:
`best[r][c] = cost[r][c] + min(best[r-1][c], best[r][c-1])`.

**Largest square of 1s.** A new kind of 2D state, "square **ending** at
`(r,c)`" (the problem 05 idea in 2D):

```
side[r][c] = 0                                              if grid[r][c] == '0'
side[r][c] = 1 + min(side[r-1][c], side[r][c-1], side[r-1][c-1])   otherwise
```

The square of 1s whose bottom-right corner is `(r,c)` can only be as big as
the smallest of the three squares touching it from above, left and
diagonal, plus one. Draw it on paper to convince yourself; this is a
standard interview question.

### Memory and layout

You only ever read the current row and the row above, so one `std::vector`
of length `C` is enough: when you overwrite `row[c]`, its old value is
"the cell above" and `row[c-1]` (already updated) is "the cell to the left".
For the square problem you also need the old `row[c-1]` (diagonal), so save
it in a variable before overwriting.

If you do keep the full table, prefer one flat `std::vector<T>(R * C)`
indexed `r * C + c` over `vector<vector<T>>`: one allocation, contiguous
memory, better cache behaviour. That matters in HFT code and in benchmarks.

## Task

Implement in `grid_paths.hpp`:

| Function | Returns |
|---|---|
| `unique_paths(rows, cols)` | number of right/down paths, exact. `1 <= rows, cols <= 30`. |
| `unique_paths_with_obstacles(grid)` | paths avoiding `'#'` cells (`'.'` is free), modulo 1 000 000 007. Up to 1000 × 1000. |
| `min_path_sum(cost)` | minimum sum of cell costs along a right/down path, including both endpoints. Up to 1000 × 1000, costs in `[0, 10^9]`. |
| `largest_square(grid)` | side length of the largest all-`'1'` square in a grid of `'0'`/`'1'`. 0 if none. Up to 1000 × 1000. |

Grids are given as `std::vector<std::string>` (rows) or `std::vector<std::vector<int64_t>>`, non-empty and rectangular.

## Requirements

- O(rows · cols) time. Try O(cols) extra memory.

<details>
<summary>Hint: first row and column</summary>

Instead of special-casing `r == 0` and `c == 0`, you can allocate one extra
row and column of zeros (or "infinity" for min) as a border, and index from
1. Many people find this removes most of the off-by-one bugs.
</details>

## Going further

- Prove `unique_paths(R, C) == binomial(R + C - 2, R - 1)` and compute it that way.
- Cherry pickup (LeetCode 741): two robots at once. The state becomes `(step, r1, r2)`.

## Reading

- Ulrich Drepper, "What Every Programmer Should Know About Memory", section 6.2 (cache-friendly access), for why flat arrays and row-major loops win.
