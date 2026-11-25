# 13 · Stock trading: DP as a state machine

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★☆☆ | 05, 04, 02 | modelling a process as states + transitions, one DP variable per state, a counted dimension (transactions left), spotting when a constraint stops mattering |

## Why this matters

Trading firms love this family because it maps directly to how strategies
are modelled: you are always in some **state** (flat, long, cooling down,
with `k` trades left), and each day you choose a **transition** (buy, sell,
hold). Once you see DP as "best value for each state, updated one step at a
time", a huge number of problems become mechanical. This is the same idea as
a hidden Markov model's Viterbi algorithm, or an optimal-execution schedule.

## The idea

You get daily prices. You hold at most one share. Each problem below adds
one rule. All return the maximum profit (≥ 0).

### Unlimited trades

Two states at the end of each day:

- `flat`: best profit if you hold no share
- `held`: best profit if you hold one (cash spent on it already subtracted)

Transitions on day with price `p`:

```
flat' = max(flat, held + p)      stay flat, or sell today
held' = max(held, flat - p)      keep holding, or buy today
start: flat = 0, held = -infinity (you can't hold before day 0)
answer: flat at the end
```

Draw it as a picture with two circles and four arrows. **Every problem in
this file is that picture with more circles or labels on arrows.**

Compute `flat'` and `held'` from the *old* `flat` and `held` (use temporaries).
Here it happens not to matter, but in the cooldown version it does.

### Transaction fee

Pay `fee` on each completed round trip: put it on the sell arrow:
`flat' = max(flat, held + p - fee)`.

### Cooldown

After selling you must wait one day before buying. Add a third state:

- `cool`: you sold **today**, so tomorrow you cannot buy

```
flat' = max(flat, cool)          rested flat (yesterday flat, or yesterday sold)
held' = max(held, flat - p)      buy only from a rested flat state
cool' = held + p                 sell today
answer: max(flat, cool)
```

### At most k transactions

Now the state needs a counter: `flat[t]`, `held[t]` for `t = 0 … k` meaning
"`t` buys used so far". Buying moves from `flat[t-1]` to `held[t]`:

```
held[t]' = max(held[t], flat[t-1] - p)
flat[t]' = max(flat[t], held[t] + p)
```

That's O(n·k). But `k` can be 10^9. Notice: with `n` days you can make at most
`n/2` useful round trips. So if `k >= n/2`, the limit **does not matter**
and the answer is the unlimited version in O(n). Always check whether a
constraint can actually bind: it often turns an impossible table into a
trivial one.

## Task

Implement in `stock_trading.hpp`:

| Function | Returns |
|---|---|
| `max_profit_unlimited(prices)` | any number of round trips |
| `max_profit_with_fee(prices, fee)` | any number of round trips, `fee` charged per round trip |
| `max_profit_with_cooldown(prices)` | any number, but no buy the day after a sell |
| `max_profit_k_trades(prices, k)` | at most `k` round trips. `0 <= k <= 10^9`. |

`n <= 200000`, prices in `[0, 10^9]`, may be empty. For `k` trades, `n·min(k, n/2)` stays under 2·10^7 in the tests.

## Requirements

- O(n) for the first three, O(n·min(k, n/2)) for the last. O(1) or O(k) memory.

<details>
<summary>Hint: k trades, order of updates</summary>

If you update `held[t]` and `flat[t]` in place in the same day, you might
buy and sell on the same day. That's harmless (profit 0), but buying with a
`flat[t-1]` that was already updated today could chain two trades into one
day. Either use the previous day's arrays, or loop `t` from `k` down to 1,
like the downward knapsack loop in problem 09.
</details>

## Going further

- Return the actual buy/sell days for the k-trade version.
- Allow short selling (hold −1, 0 or +1 shares). How many states now?
- Read about the O(n log n) solution for k trades that does not depend on `k` (it merges valley/peak pairs with a stack and a heap).

## Reading

- LeetCode 122, 714, 309 and 188 are these four problems.
- Search "Viterbi algorithm": the same state-machine DP, used for decoding hidden Markov models (common in quant research).
