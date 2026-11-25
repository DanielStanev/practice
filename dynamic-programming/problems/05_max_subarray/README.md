# 05 · Maximum subarray: "best ending here"

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★☆☆☆ | 03, 04 | the "ending at i" state, Kadane's algorithm, answer = max over all states, carrying two values (max and min) |

## Why this matters

In problem 04 the state was "best over a prefix". For anything about
**contiguous** pieces (subarrays, substrings, runs, trading windows) the
right state is usually different: **"best piece that ends exactly at i"**.
Kadane's algorithm is the classic example and an interview staple. The
one-trade stock problem is the same idea and is asked at nearly every
trading firm.

## The idea

Find the contiguous, non-empty subarray with the largest sum.

Prefix state fails here: knowing the best subarray inside `a[0..i-1]` does
not tell you whether you can extend it with `a[i]`, because it might not
touch position `i-1`. So pin the end:

**State:** `end_here[i]` = largest sum of a subarray that **ends at index `i`**.

**Recurrence:** a subarray ending at `i` is either `a[i]` alone, or `a[i]`
appended to a subarray ending at `i-1` (and you would pick the best such one):

```
end_here[i] = a[i] + max(0, end_here[i-1])        end_here[0] = a[0]
answer      = max over all i of end_here[i]
```

Read it as: *"if what came before is negative, throw it away and restart here."*

```
a         : -2   1  -3   4  -1   2   1  -5   4
end_here  : -2   1  -2   4   3   5   6   1   5     -> answer 6  ([4, -1, 2, 1])
```

Note the answer is **not** `end_here[n-1]`. With "ending at i" states, the
answer is the best over all `i`. Forgetting this is the classic bug.

### The one-trade stock problem

Given daily prices, buy once and sell once later, maximize profit (0 if you
should not trade). Selling on day `i` is best if you bought at the minimum
price before `i`. So carry `min_so_far` and compute `prices[i] - min_so_far`
at every `i`. That is the same "best ending here" pattern: it is Kadane on the
daily price *changes*.

### Maximum product: carry two states

For products, a very negative number can become very positive after another
negative factor. So "best ending here" is not enough; also carry
**"most negative ending here"**:

```
hi[i] = max(a[i], a[i]*hi[i-1], a[i]*lo[i-1])
lo[i] = min(a[i], a[i]*hi[i-1], a[i]*lo[i-1])
```

That is the problem 02 lesson again: when one number is not enough to
predict the future, keep more numbers in the state.

## Task

Implement in `max_subarray.hpp`:

| Function | Returns |
|---|---|
| `max_subarray_sum(a)` | largest sum of a non-empty contiguous subarray |
| `max_subarray_range(a)` | `{begin, end}` (half-open, `end - begin >= 1`) of one subarray achieving that sum |
| `max_profit_one_trade(prices)` | max `prices[j] - prices[i]` over `i < j`, or 0 |
| `max_product_subarray(a)` | largest product of a non-empty contiguous subarray |

Sizes up to 300000. `a` is non-empty for the subarray functions; `prices`
may be empty. For the product, `|a[i]| <= 10` and no subarray product
exceeds 10^18 in absolute value.

## Requirements

- O(n) time, O(1) extra memory for all four.

<details>
<summary>Hint: tracking the range</summary>

Keep `start` = where the current "ending here" subarray began. When you
restart (`end_here[i-1] < 0`), set `start = i`. Whenever `end_here[i]`
beats the best so far, record `{start, i + 1}`.
</details>

## Going further

- Maximum sum of a subarray of length at least `L`.
- Maximum circular subarray sum (LeetCode 918): total minus the minimum subarray.
- 2D version: the maximum-sum rectangle in a matrix in O(rows² · cols), by running Kadane on column sums.

## Reading

- Jon Bentley, *Programming Pearls*, column 8 "Algorithm Design Techniques": the original story of this problem.
