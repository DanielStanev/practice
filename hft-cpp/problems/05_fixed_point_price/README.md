# 05 · Fixed-point Price

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★☆☆☆ | 04 (constexpr, concepts) | value types, integer overflow detection, `operator<=>`, constrained operators, hand-written parsing and formatting |

## Why this matters

Prices in trading systems are almost never `double`. Exchanges publish them
as scaled integers (ITCH uses 4 implied decimals), comparisons must be exact
(is my order at the best bid or one tick behind?), and rounding has to be
deliberate. A strongly typed fixed-point `Price` catches unit mistakes at
compile time and costs exactly one `int64_t`. Problems 10, 15, 17, 18 and 20
all use integer prices.

## Task

Implement `hft::Fixed<Decimals>` in `fixed_point.hpp`. `hft::Price` is `Fixed<4>`.

## Requirements

1. **Representation.** One `int64_t` of 10<sup>-Decimals</sup> units.
   `sizeof(Price) == 8`, trivially copyable. `kScale` is computed at compile time.
2. **No implicit conversions** from integers, between scales, or to/from
   floating point. `from_raw` and `from_integer` are the explicit ways in.
3. **`parse`** accepts `[+-]digits[.digits]` with 1 to `Decimals` fractional
   digits and rejects everything else (empty, leading `.`, trailing `.`,
   whitespace, exponents, too many decimals). It detects overflow, accepts
   the full `int64_t` range including `INT64_MIN`, and is `constexpr`.
   No `strtod`, no `double`.
4. **`to_string`** always prints exactly `Decimals` fractional digits, handles
   negatives and `INT64_MIN`. `parse(to_string(x)) == x` for every `x`.
5. **Arithmetic.** `+`, `-`, unary `-`, `+=`, `-=`; `Price * integer` and
   `integer * Price` (constrained with `std::integral`). `Price * Price` and
   `Price * double` must not compile.
6. **Checked arithmetic.** `checked_add`, `checked_mul` return `nullopt` on overflow.
7. **`round_to_tick(tick, mode)`** with `Down` (toward −∞), `Up` (toward +∞)
   and `Nearest` (ties away from zero). Careful: C++ integer division
   truncates toward zero.
8. **Comparisons** via a defaulted `operator<=>`.

## Running the tests

```bash
./run_tests.sh
```

<details>
<summary>Hints</summary>

- GCC and Clang have `__builtin_add_overflow`, `__builtin_mul_overflow` and
  `__builtin_sub_overflow`, which work in constant expressions.
- To accept `INT64_MIN`, accumulate the parsed value as a *negative* number
  (there is one more negative `int64_t` than positive) and negate at the end.
- For `to_string`, take the magnitude as `uint64_t` (`0 - (uint64_t)raw`)
  so negating `INT64_MIN` is not undefined behaviour.
</details>

## Going further

- Add `std::formatter<Fixed<D>>` so `std::format("{}", price)` works.
- Write `to_chars` into a caller-provided buffer with no allocation, using
  `count_digits` from problem 04, and benchmark it against `std::to_string`.
- Add a `Quantity` strong type and make `Price * Quantity` return a
  `Notional` type.

## Reading

- cppreference: [default comparisons](https://en.cppreference.com/w/cpp/language/default_comparisons),
  [`std::integral`](https://en.cppreference.com/w/cpp/concepts/integral)
- "What Every Computer Scientist Should Know About Floating-Point Arithmetic" (Goldberg)
- Jonathan Müller, "Strong typedefs" (foonathan.net blog series)
