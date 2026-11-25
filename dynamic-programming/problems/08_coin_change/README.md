# 08 · Coin change: unbounded choices, and why loop order matters

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★☆☆☆ | 02, 03 | unbounded knapsack, "impossible" states, combinations vs permutations, how loop order changes what you count |

## Why this matters

Coin change is the gateway to knapsack (problem 09). It also contains the
single most instructive DP bug: swapping two `for` loops silently changes
the answer from "combinations" to "orderings". If you understand *why*,
you understand DP state much better than most candidates.

## The idea

Coins come in denominations `coins[]`, unlimited supply of each.

### Fewest coins

`best[a]` = fewest coins summing to exactly `a`. The last coin used was
some `c`, leaving `a - c`:

```
best[0] = 0
best[a] = 1 + min over coins c <= a of best[a - c]        (or "impossible")
```

Some amounts can't be made (coins `{2}`, amount `3`). Represent
"impossible" explicitly (`-1`, or a big "infinity" that you check before
adding 1). Greedy (always take the biggest coin) is **wrong**: with coins
`{1, 3, 4}` and amount 6, greedy gives `4+1+1` (3 coins), DP finds `3+3`.

### Counting: two different questions

Coins `{1, 2}`, amount 3. Is `1+2` the same as `2+1`?

- **Combinations** (order doesn't matter): `{1,1,1}`, `{1,2}` → **2**
- **Permutations** (order matters): `1+1+1`, `1+2`, `2+1` → **3**

Permutations is climbing stairs (problem 02): `ways[a] = Σ ways[a - c]`.
Loop amounts on the outside, coins on the inside:

```
for a in 1..amount:
    for c in coins: ways[a] += ways[a - c]
```

For combinations, count each multiset once by deciding coins **in a fixed
order**: first decide how many of coin 0, then coin 1, and so on. Loop coins
on the **outside**:

```
ways[0] = 1
for c in coins:
    for a in c..amount: ways[a] += ways[a - c]
```

After processing the first `k` coins, `ways[a]` = number of ways to make `a`
using only those `k` coins. Adding coin `c` on top: either use no `c` (the
old value), or at least one `c` (`ways[a - c]`, already updated, so it may
contain more `c`s). Since coins are always added in the same order, `2+1` and
`1+2` are the same sequence and only counted once.

Same two lines, swapped, different problem. Run both by hand for
`{1, 2}`, amount 3 until it clicks.

## Task

Implement in `coin_change.hpp`:

| Function | Returns |
|---|---|
| `min_coins(coins, amount)` | fewest coins summing to `amount`, or -1. `amount = 0` → 0. |
| `count_combinations(coins, amount)` | number of multisets of coins summing to `amount`, mod 1 000 000 007 |
| `count_permutations(coins, amount)` | number of ordered sequences of coins summing to `amount`, mod 1 000 000 007 |

`1 <= coins.size() <= 100`, coins are distinct, `1 <= coins[i] <= 100000`, `0 <= amount <= 100000`.

## Requirements

- O(coins · amount) time, O(amount) memory.

<details>
<summary>Hint: is the inner loop going up or down?</summary>

In `count_combinations` the inner loop goes **up** (`a = c … amount`), so
`ways[a - c]` already includes uses of coin `c`. That is what makes the
supply unlimited. In problem 09 (each item at most once) the loop goes
**down** instead. Keep this in mind.
</details>

## Going further

- Return the actual coins for `min_coins`.
- Each coin `i` has a limited count `k_i` (bounded knapsack). The simple fix
  is to treat it as `k_i` separate coins; the clever one uses binary splitting
  (1, 2, 4, … copies).

## Reading

- "Knapsack problems" in *Competitive Programmer's Handbook* by Antti Laaksonen (free PDF), chapter 7.
