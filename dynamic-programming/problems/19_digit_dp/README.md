# 19 · Digit DP: counting numbers up to 10^18 without looking at them

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★★☆ | 02, 15 | DP over the digits of a bound, the "tight" flag, the "started" flag for leading zeros, `f(hi) - f(lo - 1)`, memoizing a recursive solution with several state variables |

## Why this matters

"How many numbers in `[lo, hi]` have property P?" with `hi` up to 10^18
can't be answered by looping. Digit DP builds the numbers digit by digit and
only remembers what P needs. It's a favourite in harder interviews and
contests because the state design is subtle: you have to say exactly what
"the rest of the number" needs to know about the prefix.

## The idea

### Step 1: ranges become prefixes

Let `F(N)` = how many `x` in `[0, N]` have property P. Then the count in
`[lo, hi]` is `F(hi) - F(lo - 1)` (with `F(-1) = 0`). Now there's only an
upper bound to worry about.

### Step 2: build numbers digit by digit under the bound

Write `N` as digits, most significant first: `N = 3 5 2`. A number `x <= N`
with the same number of digit slots (padding with leading zeros: `007`) is
chosen left to right. At each position, the digits you may use depend on
whether the prefix so far **equals** `N`'s prefix:

- **tight** (prefix equal so far): this digit can be `0 … N[pos]`. Choosing
  exactly `N[pos]` keeps you tight; anything smaller makes you loose.
- **loose** (already smaller): this digit can be anything `0 … 9`, and you
  stay loose.

### Step 3: remember what the property needs

For "digit sum equals S" the remainder only needs the sum so far:

```
count(pos, sum, tight) = number of ways to fill positions pos.. so that the
                         total digit sum is S, given the prefix has digit sum
                         `sum` and tightness `tight`
count(len, sum, _) = (sum == S) ? 1 : 0
count(pos, sum, tight) = Σ over allowed d of count(pos+1, sum+d, tight && d == N[pos])
```

Memoize on `(pos, sum, tight)`: 19 × 163 × 2 states, tiny. (Only the
`tight == false` states are reused much; many people memoize only those.)

### Step 4: leading zeros, when they matter

For digit sum, leading zeros are harmless (they add 0). For "no two adjacent
digits are equal", they're not: `007` padded looks like it has `00`, but the
number is `7`. Add a **started** flag: until the first non-zero digit, zeros
are padding and there is no previous digit. The state becomes
`(pos, prev_digit, tight, started)`.

Counting digit occurrences (how many times does digit `d` appear in all the
numbers from `lo` to `hi`?) is the same, except that each completed number
contributes **how many times `d` appeared**, not 1. Carry the count in the
state, or (nicer) return a pair `{numbers, occurrences}` from each state and
add `d`'s contribution when you place it. For `d = 0`, padding zeros must not
count. The number `0` itself is written `"0"` and contains one zero.

Recursion is natural here (depth is only ~19), so a memoized recursive
function is the cleanest implementation.

## Task

Implement in `digit_dp.hpp`. All ranges are inclusive with `0 <= lo <= hi <= 10^18`.

| Function | Returns |
|---|---|
| `count_digit_sum(lo, hi, s)` | how many `x` in `[lo, hi]` have decimal digit sum exactly `s` (`0 <= s <= 200`) |
| `count_no_adjacent_equal(lo, hi)` | how many `x` in `[lo, hi]` have no two equal adjacent digits in their decimal form (`0`–`9` all qualify, `11` does not, `121` does) |
| `count_digit_occurrences(lo, hi, d)` | total number of times digit `d` (0–9) is written when writing every number in `[lo, hi]` in decimal |

## Requirements

- Each call O(digits × state size × 10). Thousands of calls must finish in well under a second.

<details>
<summary>Hint: a tidy structure</summary>

```cpp
std::int64_t count_upto(std::int64_t N) {        // [0, N]
    if (N < 0) return 0;
    std::string digits = std::to_string(N);
    // memo[pos][...][...] = -1 means unknown; only use it when !tight
    // auto go = [&](auto& self, int pos, ..., bool tight, bool started) -> std::int64_t { ... };
    return go(go, 0, ..., true, false);
}
```
</details>

## Going further

- Count numbers in `[lo, hi]` divisible by the sum of their digits (try every target sum, track the remainder).
- Find the k-th smallest number with a property: walk the digits, using the counts to pick each digit.
- "Stepping numbers": adjacent digits differ by exactly 1.

## Reading

- Codeforces blog: "Digit DP" tutorials (several good ones, search "digit dp tutorial codeforces").
- LeetCode 233 (Number of Digit One), 902, 1012.
