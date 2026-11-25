# 02 · Climbing stairs: counting with "what was the last move?"

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★☆☆☆☆ | 01 | counting DP, deriving a recurrence from the last decision, modular arithmetic, adding a dimension to the state |

## Why this matters

Problem 01 handed you the recurrence. Real problems do not. This problem
teaches the most useful way to *find* one: **look at the last step.**

## The idea

You are at the bottom of a staircase with `n` steps. Each move climbs 1 or 2
steps. How many different sequences of moves reach the top exactly?

Let `ways[k]` = number of ways to stand on step `k`. Think about the **last
move** that landed you on step `k`. It was either

- a 1-step from step `k-1` (and there are `ways[k-1]` ways to be there), or
- a 2-step from step `k-2` (and there are `ways[k-2]` ways to be there).

Those two groups never overlap (their last moves differ) and together they
cover everything, so

```
ways[k] = ways[k-1] + ways[k-2]
```

Base case: `ways[0] = 1`. There is exactly one way to be at the bottom: do
nothing. Beginners often set it to 0, which makes every answer 0. When
counting, "the empty plan" counts as one plan.

```
k     : 0 1 2 3 4 5
ways  : 1 1 2 3 5 8
```

Yes, it is Fibonacci again. That is the point: many problems are the same
recurrence in disguise, and the "last move" argument is how you see it.

### Allowed steps as a set

If the allowed step sizes are `{1, 3, 5}`, the same argument gives
`ways[k] = ways[k-1] + ways[k-3] + ways[k-5]` (skipping negative indices).
Counts explode quickly, so you return them **modulo 1 000 000 007**. Take
`% MOD` after every addition so nothing overflows; with `MOD < 2^30`, adding
two reduced values always fits in `int64_t`.

### When the last move is not enough: add to the state

New rule: moves are 1 or 2 steps, but **you may not take two 2-steps in a
row**. Now `ways[k]` is not enough, because whether you may take a 2-step
depends on *how you arrived*. The fix is the second big DP habit:

> If the future depends on something about the past, put that something into the state.

Use two tables:

- `end1[k]` = ways to reach step `k` whose last move was a 1-step
- `end2[k]` = ways to reach step `k` whose last move was a 2-step

and work out each from the previous ones. Answer: `end1[n] + end2[n]`
(careful with `n = 0`).

## Task

Implement in `climbing_stairs.hpp`:

| Function | Returns |
|---|---|
| `count_ways(n)` | ways to climb `n` steps with moves {1, 2}. `0 <= n <= 90`, exact `uint64_t`. |
| `count_ways_with_steps(n, steps)` | ways with the given distinct positive step sizes, modulo 1e9+7. `0 <= n <= 200000`, `1 <= steps.size() <= 10`. |
| `count_ways_no_double_two(n)` | ways with moves {1, 2} where two 2-steps are never consecutive. `0 <= n <= 80`, exact. |

## Requirements

- O(n) or O(n · steps) time. No recursion without memoization.
- The tests compare against brute force for small `n`, so off-by-one errors in base cases will show up.

<details>
<summary>Hint: the no-double-two recurrence</summary>

`end1[k]`: the previous move could have been anything, so `end1[k] = end1[k-1] + end2[k-1]`.
`end2[k]`: the previous move must have been a 1-step (or there was no previous move).
Treat step 0 as "arrived by a 1-step" (`end1[0] = 1`, `end2[0] = 0`), because
"no move yet" allows a 2-step just like a 1-step does.
</details>

## Going further

- Rewrite all three with O(1) memory (`count_ways_with_steps` needs O(max step)).
- Count ways to tile a 2 × n board with 1 × 2 dominoes. Then a 3 × n board (much harder: the state is the shape of the jagged edge).

## Reading

- Steven Halim, *Competitive Programming 4*, section 3.5 (DP), "classical examples".
- cp-algorithms.com: "Introduction to Dynamic Programming".
