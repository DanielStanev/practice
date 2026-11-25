# 15 · Probability and expected value: DP the way quants use it

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★☆☆ | 02, 08, 13 | probabilities instead of counts, expected values, optimal stopping (backward induction), states that loop back to themselves, floating-point care |

## Why this matters

Quant trading interviews are full of dice, coins and games: "what is this
game worth?", "should you re-roll?", "how long until…?". Almost all of them
are DP over states with probabilities. The same backward induction prices
American options on a binomial tree. If you're interviewing at HFT/prop
shops, this is probably the most directly useful file in the repo.

## The idea

### Counting → probability

The number of ways `n` dice show sum `s` is a counting DP just like coin
permutations (problem 08): `ways[d][s] = Σ_f ways[d-1][s-f]`. Divide by
`faces^n` for a probability, or, better, work with probabilities directly
(`prob[d][s] = Σ_f prob[d-1][s-f] / faces`) so nothing overflows: with 100
dice the counts are around 6^100.

### Expected value and optimal stopping

Game: roll a fair d6, you may stop and take the face value in dollars, or
re-roll (up to `k` rolls in total; the last roll you must accept). What is
the game worth with optimal play?

Work **backwards** from the end (backward induction):

```
V(1) = 3.5                      last roll: just take its expected value
V(k) = E[ max(X, V(k-1)) ]      roll; keep X if it beats the value of continuing
     = (1/6) · Σ_{x=1..6} max(x, V(k-1))
```

`V(2) = (1/6)(3.5+3.5+3.5+4+5+6) = 4.25`: with two rolls, keep 4, 5, 6 and
re-roll 1, 2, 3. The state is "rolls remaining"; the recurrence takes the
best decision at each state. That's the whole of optimal stopping.

### Expected time, and states that point to themselves

Board squares `0 … n`. From square `i` you roll a d6 and move forward; if
the move would pass `n` you stay where you are (the roll is wasted). Expected
number of rolls to land exactly on `n`?

`E[n] = 0`. For `i < n`, with `c` = number of faces that don't overshoot:

```
E[i] = 1 + (1/6) · ( Σ_{valid f} E[i+f]  +  (6 - c) · E[i] )
```

`E[i]` appears on both sides (the wasted rolls). That's fine. Solve the
linear equation for `E[i]`:

```
E[i] · (c/6) = 1 + (1/6) Σ_{valid f} E[i+f]
E[i]         = (6 + Σ_{valid f} E[i+f]) / c
```

Fill from `i = n-1` down to 0. Self-loops are common in Markov-chain
problems; isolate the unknown and divide.

### Runs: probability with a "memory" state

Probability of seeing at least one run of `k` heads in a row in `n` flips
of a coin with `P(heads) = p`. The future depends on the length of the
current run of heads, so put it in the state (the problem 02 lesson):

```
P[i][r] = probability that after i flips there has been no k-run yet
          and the current run of heads has length r   (0 <= r < k)
heads: P[i+1][r+1] += p · P[i][r]       (if r+1 == k, that mass has "succeeded")
tails: P[i+1][0]   += (1-p) · P[i][r]
answer = 1 - Σ_r P[n][r]
```

Computing the probability of the **complement** (no run) is often easier,
another classic trick.

### Floating point

Use `double`. Tests compare with a tolerance of 1e-9. Adding many tiny
probabilities is fine at these sizes; don't compute `6^100` and divide.

## Task

Implement in `probability_dp.hpp`:

| Function | Returns |
|---|---|
| `dice_sum_probability(n, faces, target)` | P(sum of `n` fair dice with faces `1..faces` equals `target`). `n <= 200`, `faces <= 20`, any `target`. |
| `optimal_dice_game_value(rolls)` | value of the re-roll game with a d6 and at most `rolls >= 1` rolls |
| `expected_rolls_to_finish(n)` | expected d6 rolls to land exactly on square `n` from square 0, as described. `0 <= n <= 1000000`. |
| `prob_run_of_heads(n, k, p)` | P(at least one run of `k` consecutive heads in `n` flips). `n <= 20000`, `1 <= k <= 200`. |

## Requirements

- `dice_sum_probability`: O(n · n·faces · faces) is fine. The rest O(n), O(n), O(n·k).

<details>
<summary>Hint: optimal game for large k</summary>

`V(k)` approaches 6 but never reaches it. Each step is O(1); no table needed.
</details>

## Going further

- Gambler's ruin: start with `i` dollars, win/lose 1 with probability `p`, stop at 0 or `N`. P(reach N)? (A linear system with a closed form; try value iteration as a DP.)
- Price an American put on a binomial tree with backward induction. Same recurrence shape as the dice game: `max(exercise now, discounted expected continuation)`.
- The "Ace of spades" game: deal cards one by one; say "stop" whenever; you win if the next card is red. What's the best strategy? (Surprise.)

## Reading

- Xinfeng Zhou, *A Practical Guide to Quantitative Finance Interviews* ("the green book"), chapters 4 and 5.
- Timothy Crack, *Heard on the Street*.
- Mark Joshi, *Quant Job Interview Questions and Answers*.
