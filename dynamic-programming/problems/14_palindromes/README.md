# 14 · Palindromes: your first interval DP

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★☆☆ | 07, 11 | interval state `dp[i][j]` over a substring, filling by increasing length, combining an interval table with a prefix DP |

## Why this matters

So far every state was a prefix (or two prefixes). Some problems are about
an arbitrary **piece in the middle**: `s[i..j]`. Those are interval DPs, and
they need a new fill order. Palindromes are the gentlest introduction;
problem 16 uses the same pattern for harder questions.

## The idea

### Is `s[i..j]` a palindrome?

```
pal[i][j] = (s[i] == s[j]) && (j - i < 2 || pal[i+1][j-1])
```

An interval is a palindrome if its ends match and the inside is one. The
inside `[i+1, j-1]` is **shorter**, so fill by **increasing length**:

```
for len in 1..n:
    for i in 0..n-len:
        j = i + len - 1
        ...uses only intervals of length < len...
```

(Equivalently: `i` from `n-1` down to 0 and `j` from `i` up. Work out why
that order also works. Being able to derive valid orders yourself is the
skill being trained here.)

With `pal[][]`, counting palindromic substrings is just counting `true`
cells, and the longest palindromic substring is the longest `true` interval.
(You can also do both with "expand around each centre" in O(n²) time and O(1)
memory. Try both.)

### Longest palindromic subsequence

```
L[i][j] = longest palindromic subsequence inside s[i..j]
L[i][i] = 1
L[i][j] = L[i+1][j-1] + 2                 if s[i] == s[j]
        = max(L[i+1][j], L[i][j-1])       otherwise
```

Look at that "otherwise" line next to LCS (problem 11). It is the same idea:
drop one end or the other. (In fact LPS(s) = LCS(s, reverse(s)).)

### Minimum cuts: interval table + prefix DP

Cut `s` into pieces that are all palindromes, using as few cuts as possible.
`"aab"` → `"aa" | "b"`: 1 cut.

Use a **prefix** DP on top of the interval table:

```
cuts[j] = fewest pieces for the prefix s[0..j-1]
cuts[0] = 0
cuts[j] = min over i < j with pal[i][j-1] of cuts[i] + 1
answer  = cuts[n] - 1         (pieces minus one)
```

This is word break (problem 06) where the "dictionary" is "all palindromic
substrings". Combining two DPs like this (precompute one table, use it in
another) is very common.

### Memory

`n = 3000` means 9 million cells. A `std::vector<char>` or
`std::vector<std::uint8_t>` for `pal` is 9 MB; `vector<vector<bool>>` is slow and
`vector<vector<int>>` is 36 MB in 3000 separate allocations. Use one flat array.

## Task

Implement in `palindromes.hpp`:

| Function | Returns |
|---|---|
| `longest_palindromic_substring(s)` | one longest palindromic substring (any, if tied). `n <= 5000`. |
| `count_palindromic_substrings(s)` | number of (start, end) pairs whose substring is a palindrome. `n <= 5000`. |
| `longest_palindromic_subsequence(s)` | its length. `n <= 3000`. |
| `min_palindrome_cuts(s)` | fewest cuts so every piece is a palindrome. `1 <= n <= 3000`. |

## Requirements

- O(n²) time for all four.

<details>
<summary>Hint: O(n) memory for the subsequence</summary>

Loop `i` from `n-1` down to 0 and `j` from `i+1` up. Row `i` only needs row
`i+1`, so two rows (or one row plus a saved diagonal) are enough.
</details>

## Going further

- Manacher's algorithm: longest palindromic substring in O(n).
- Minimum insertions to make a string a palindrome (hint: it's `n - LPS`).
- Palindrome partitioning with O(n) memory: expand around centres and update `cuts` as you go.

## Reading

- cp-algorithms.com: "Manacher's Algorithm".
- Search "interval DP" (区间DP in Chinese competitive programming material, which has the most examples).
