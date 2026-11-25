# 12 · Edit distance: three moves per cell, and turning a table into a script

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★☆☆ | 11 | three-way transitions, non-trivial base cases (first row and column), producing a list of operations from the table, weighted costs |

## Why this matters

Levenshtein distance powers spell checkers, fuzzy symbol matching
("did you mean AAPL?"), diffing config files, and DNA alignment. It's the
most common "hard-ish" 2D DP in interviews, and it tests whether you really
understand base cases.

## The idea

Turn string `a` into string `b` using single-character operations:
**insert**, **delete**, **replace**. Minimize the number of operations.
`"horse" → "ros"` takes 3: replace h→r, delete r, delete e.

**State:** `E[i][j]` = cost to turn the prefix `a[0..i-1]` into `b[0..j-1]`.

**Recurrence:** look at the last character of each prefix:

- `a[i-1] == b[j-1]`: nothing to do for it: `E[i-1][j-1]`
- otherwise take the cheapest of:
  - **replace** `a[i-1]` with `b[j-1]`: `E[i-1][j-1] + 1`
  - **delete** `a[i-1]`: `E[i-1][j] + 1`
  - **insert** `b[j-1]` at the end: `E[i][j-1] + 1`

**Base cases** are not zero this time:

- `E[i][0] = i`: turning `i` characters into nothing takes `i` deletions
- `E[0][j] = j`: building `j` characters from nothing takes `j` insertions

Getting these wrong (e.g. initializing everything to 0, as in LCS) is the
classic bug. Always ask: *what is the true answer when one side is empty?*

```
          ""  r  o  s
     ""    0  1  2  3
     h     1  1  2  3
     o     2  2  1  2
     r     3  2  2  2
     s     4  3  3  2
     e     5  4  4  3
```

### Turning the table into a script

The table tells you the cost. To get **what to do**, walk back from
`(n, m)` and, at each cell, ask which of the candidates produced its value:

- came from `(i-1, j-1)` with equal chars → `Keep`
- came from `(i-1, j-1)` with +1 → `Replace` with `b[j-1]`
- came from `(i-1, j)` → `Delete`
- came from `(i, j-1)` → `Insert` `b[j-1]`

At the borders only one move is possible (`i == 0` → inserts, `j == 0` →
deletes). Collect the moves and reverse them.

The script format used here is an **alignment**: a list of operations
applied left to right while walking through `a` once:

| Op | Effect on the output | Consumes a char of `a`? |
|---|---|---|
| `Keep` | append the current char of `a` | yes |
| `Replace(c)` | append `c` | yes |
| `Delete` | nothing | yes |
| `Insert(c)` | append `c` | no |

Applying it must produce exactly `b`, and the number of non-`Keep`
operations must equal the distance. The tests apply your script to check it.

### Weighted costs

If insert, delete and replace have different costs, change `+1` to `+cost`
and the base cases to `i * del_cost` and `j * ins_cost`. Note that a replace
might now cost more than delete + insert; the `min` handles that for you.

## Task

Implement in `edit_distance.hpp`:

| Function | Returns |
|---|---|
| `edit_distance(a, b)` | Levenshtein distance. Lengths up to 5000, O(min(n,m)) memory. |
| `edit_script(a, b)` | a minimal alignment script as above. Lengths up to 2000. |
| `edit_distance_weighted(a, b, ins, del, rep)` | minimal total cost with these operation costs (each `>= 0`). Lengths up to 5000. |

## Requirements

- O(n·m) time.

<details>
<summary>Hint: rolling rows with non-zero base cases</summary>

With one row, `row[j]` starts as `E[0][j] = j`. At the start of row `i`,
remember the old `row[0]` as the diagonal, then set `row[0] = i`. As in
problem 07, save `row[j]` before overwriting it: it is the next diagonal.
</details>

## Going further

- Damerau-Levenshtein: also allow swapping two adjacent characters.
- If you only care whether the distance is at most `k`, you only need cells
  with `|i - j| <= k`: O((n+m)·k). That band trick is used in real fuzzy matchers.

## Reading

- Dan Gusfield, *Algorithms on Strings, Trees and Sequences*, chapter 11.
- Wikipedia: "Wagner-Fischer algorithm".
