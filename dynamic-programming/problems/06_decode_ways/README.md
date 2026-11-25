# 06 · Decode ways and word break: DP over string prefixes

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★☆☆☆ | 02, 04 | DP over string prefixes, transitions with validity conditions, boolean DP, min-count DP, `std::string_view` and hash sets |

## Why this matters

Parsing is everywhere in trading systems (FIX, ITCH, config files) and
"in how many ways / can I / in how few pieces can this string be split" is a
very common interview family. All of them use the same prefix state.

## The idea

**Decode ways.** Letters are encoded `A → "1"`, `B → "2"`, … `Z → "26"`.
Given a digit string, count the ways to decode it. `"226"` → `"2 2 6"`,
`"22 6"`, `"2 26"`: 3 ways.

**State:** `ways[i]` = number of ways to decode the prefix `s[0 … i-1]`.

**Recurrence:** what was the *last letter* of the decoding of that prefix?
It used either the last 1 digit or the last 2 digits:

- 1 digit `s[i-1]` is a valid letter iff it is `'1'…'9'` → add `ways[i-1]`
- 2 digits `s[i-2..i-1]` are a valid letter iff they form `10 … 26` → add `ways[i-2]`

```
ways[0] = 1          (empty prefix: one way, the empty decoding)
answer  = ways[n]
```

This is climbing stairs (problem 02) where some steps are forbidden.
Example `"2101"`:

```
i      : 0  1("2")  2("21")  3("210")  4("2101")
ways   : 1  1       2        1         1
```

`ways[3]`: last digit `'0'` alone is invalid, `"10"` is valid → `ways[1] = 1`.
Zeros are where people get it wrong, so the tests have many.

**Word break.** Given a dictionary, can `s` be split into dictionary words?
Same prefix state, but boolean:

```
ok[0] = true
ok[i] = OR over every word w that s[0..i-1] ends with:  ok[i - |w|]
```

Implementation: for each `i`, try every length `L` from 1 to the longest
word, and check if `s.substr(i-L, L)` is in an `std::unordered_set`. With
`std::string_view` you can look up without allocating (use a set of
`std::string_view` that points into the dictionary, or C++20 heterogeneous
lookup).

**Fewest words.** Same structure again with `min` instead of `OR`:
`best[i] = 1 + min best[i - |w|]`, "infinity" when impossible (problem 03).

Notice how three different questions (count, possible?, minimum) are the
same table with a different way of combining: `+`, `OR`, `min`. Spotting
this is a big step towards DP fluency.

## Task

Implement in `decode_ways.hpp`:

| Function | Returns |
|---|---|
| `num_decodings(s)` | number of decodings modulo 1 000 000 007. `s` holds only digits, `0 <= |s| <= 200000`. Empty string → 1. |
| `word_break(s, dict)` | whether `s` splits into words from `dict` (words may be reused) |
| `min_words(s, dict)` | fewest words in such a split, or -1 if none. Empty `s` → 0. |

For the word functions: `|s| <= 100000`, `dict.size() <= 1000`, each word length 1 to 20, lowercase.

## Requirements

- `num_decodings` in O(n). Word functions in O(n · maxWordLength) hash lookups.

<details>
<summary>Hint: zeros</summary>

`"0"`, `"00"`, `"30"`, `"100"` all have 0 decodings. Your two rules already
handle them if you apply them exactly: a single `'0'` is never valid, and a
two-digit chunk starting with `'0'` (like `"05"`) is never valid.
</details>

## Going further

- Decode ways II (LeetCode 639) where `'*'` can be any digit 1-9.
- Return *all* sentences for word break (LeetCode 140). Why is that exponential in the worst case even with memoization?

## Reading

- cppreference: `std::string_view`, `std::unordered_set` heterogeneous lookup (`is_transparent`, C++20).
