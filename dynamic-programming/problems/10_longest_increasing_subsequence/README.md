# 10 · Longest increasing subsequence: from O(n²) to O(n log n)

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★☆☆ | 03, 05 | subsequence (not subarray) DP, O(n²) "look at every earlier j", speeding up a DP with a clever auxiliary array + binary search, reduction by sorting |

## Why this matters

LIS is the first problem where the obvious DP is too slow and you need to
make the DP itself faster. That skill (keep the recurrence, change the data
structure) is what separates good DP from great DP, and problem 20 is all
about it. LIS also appears in disguise: box stacking, scheduling, and
"Russian doll envelopes", which you'll solve here.

## The idea

A **subsequence** keeps the original order but may skip elements
(`[3, 7]` is a subsequence of `[3, 1, 7]`; a subarray must be contiguous).
Find the longest **strictly** increasing subsequence.

### O(n²)

**State:** `len[i]` = length of the longest increasing subsequence that
**ends at index `i`** (the problem 05 "ending here" idea).

**Recurrence:** the element before `a[i]` in that subsequence is some
`a[j]` with `j < i` and `a[j] < a[i]`, or there is none:

```
len[i] = 1 + max( len[j] for j < i with a[j] < a[i] )   (or 1 if no such j)
answer = max over i of len[i]
```

```
a   : 10  9  2  5  3  7  101  18
len :  1  1  1  2  2  3    4   4     -> 4  (e.g. 2, 3, 7, 18)
```

### O(n log n): patience sorting

With `n = 200000`, n² is 4·10^10: too slow. Keep a different array:

> `tails[k]` = the **smallest possible last element** of an increasing subsequence of length `k+1` seen so far.

Smaller tails are better: they are easier to extend. `tails` is always
strictly increasing (prove it to yourself). For each `x`:

- find the first `tails[k] >= x` (`std::lower_bound`)
- if none, `x` extends the longest subsequence: `tails.push_back(x)`
- otherwise `x` is a better (smaller) tail for length `k+1`: `tails[k] = x`

The answer is `tails.size()`. Watch it on the example:

```
x=10: [10]
x=9 : [9]
x=2 : [2]
x=5 : [2, 5]
x=3 : [2, 3]
x=7 : [2, 3, 7]
x=101:[2, 3, 7, 101]
x=18: [2, 3, 7, 18]      -> length 4
```

Careful: `tails` is **not** itself an LIS in general. For reconstruction,
also remember for each `i` the index of the element that was in
`tails[k-1]` when `a[i]` was placed at `tails[k]` (its parent), and the
index currently stored at each `tails` position. Then walk parents back from
the last tail.

`lower_bound` gives *strictly* increasing. `upper_bound` would give
non-decreasing. Know which one you need.

### Russian doll envelopes

Envelope `(w, h)` fits in `(W, H)` if `w < W` **and** `h < H`. How many can
you nest? Sort by width ascending; then you want an LIS on heights. But two
envelopes with the same width can't nest, and an LIS on heights might pick
both. Trick: for equal widths, sort heights **descending**, so an increasing
subsequence can never take two of them. Then run the O(n log n) LIS on heights.

## Task

Implement in `lis.hpp`:

| Function | Returns |
|---|---|
| `lis_length_quadratic(a)` | LIS length, the O(n²) way. Tested only with `n <= 2000`. |
| `lis_length(a)` | LIS length in O(n log n). `n <= 300000`. |
| `lis_sequence(a)` | the values of one longest strictly increasing subsequence, in order. O(n log n). |
| `max_envelopes(env)` | max number of nested envelopes. `n <= 200000`. |

Values are any `int` (including negative). Empty input → 0 / empty.

## Requirements

- O(n log n) for the last three. The big tests will time out at O(n²).

<details>
<summary>Hint: reconstruction bookkeeping</summary>

Keep `tail_idx[k]` = index in `a` of the element stored at `tails[k]`, and
`parent[i] = (k > 0 ? tail_idx[k-1] : -1)` at the moment you place `a[i]`
at position `k`. At the end, start from `tail_idx.back()` and follow `parent`.
</details>

## Going further

- Count the number of distinct longest increasing subsequences (LeetCode 673).
- Longest *bitonic* subsequence: increasing then decreasing.
- Explain why the minimum number of strictly decreasing subsequences that
  cover the array equals the LIS length (Dilworth's theorem / patience sorting).

## Reading

- cp-algorithms.com: "Longest increasing subsequence".
- David Aldous and Persi Diaconis, "Longest increasing subsequences: from patience sorting to the Baik-Deift-Johansson theorem" (1999), for the beautiful math behind it.
