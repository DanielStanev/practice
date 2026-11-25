# 11 · Longest common subsequence: DP over two strings

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★☆☆ | 05, 07, 10 | 2D state over two sequences `dp[i][j]`, matching vs skipping, rolling rows, reconstruction by walking the table, substring vs subsequence |

## Why this matters

"Compare two sequences" problems (diff tools, DNA alignment, reconciling two
trade logs, edit distance next) all use the same table: one index into each
sequence. Get this one solid and problem 12 is a small step.

## The idea

Given strings `a` (length n) and `b` (length m), find the longest string
that is a subsequence of both. `LCS("ABCBDAB", "BDCABA")` has length 4
(`"BCBA"`, among others).

**State:** `L[i][j]` = LCS length of the prefixes `a[0..i-1]` and `b[0..j-1]`.

**Recurrence:** look at the last characters of the two prefixes.

- If `a[i-1] == b[j-1]`: use that character as the last character of the
  LCS: `L[i][j] = L[i-1][j-1] + 1`.
- Otherwise, at least one of them is not in the LCS; drop one or the other:
  `L[i][j] = max(L[i-1][j], L[i][j-1])`.

```
L[0][j] = L[i][0] = 0          (empty prefix)
answer  = L[n][m]
```

Fill order: row by row, like problem 07 (each cell needs up, left, up-left).

```
        ""  B  D  C  A  B  A
   ""    0  0  0  0  0  0  0
   A     0  0  0  0  1  1  1
   B     0  1  1  1  1  2  2
   C     0  1  1  2  2  2  2
   B     0  1  1  2  2  3  3
   D     0  1  2  2  2  3  3
   A     0  1  2  2  3  3  4
   B     0  1  2  2  3  4  4
```

### Reconstruction

Start at `(n, m)` and walk back:

- if `a[i-1] == b[j-1]`: that character is in the LCS; go to `(i-1, j-1)`
- else go to whichever of `(i-1, j)` / `(i, j-1)` has the larger value

Collect characters and reverse. This needs the full table: O(n·m) memory.
Use a flat `std::vector<uint16_t>` (values ≤ 3000 fit), not
`vector<vector<int>>`.

### Substring vs subsequence

The longest common **substring** must be contiguous in both strings.
Changing the state to "ending at" (problem 05 again) does it:

```
S[i][j] = length of the longest common suffix of a[0..i-1] and b[0..j-1]
        = (a[i-1] == b[j-1]) ? S[i-1][j-1] + 1 : 0
answer  = max over all i, j
```

Same table shape, a different state meaning, and a different answer
location (max over everything vs the corner).

## Task

Implement in `lcs.hpp`:

| Function | Returns |
|---|---|
| `lcs_length(a, b)` | LCS length. Lengths up to 5000 each. Use O(min(n,m)) memory. |
| `lcs_string(a, b)` | one LCS. Lengths up to 3000 each. Any LCS is accepted. |
| `longest_common_substring(a, b)` | length of the longest common contiguous substring. Lengths up to 5000. |

## Requirements

- O(n·m) time.

<details>
<summary>Hint: two rows for LCS length</summary>

Keep `prev` (row `i-1`) and `cur` (row `i`), each of length `m+1`, and
`std::swap` them after each row. You can get it down to one row plus one
saved "diagonal" variable, like the square problem in 07.
</details>

## Going further

- Hirschberg's algorithm: the LCS *string* in O(n·m) time but O(n+m) memory, by divide and conquer.
- Shortest common supersequence: `n + m - LCS` long; build it from the table.
- If both strings are permutations of `1..n`, LCS reduces to LIS (problem 10) and runs in O(n log n). Work out why.

## Reading

- CLRS chapter 14.4 (15.4 in older editions), "Longest common subsequence".
- The `diff` algorithm: Eugene Myers, "An O(ND) Difference Algorithm and Its Variations" (1986).
