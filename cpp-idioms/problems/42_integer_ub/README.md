# 42 · Integer arithmetic without undefined behaviour

| Group | Difficulty | Builds on |
|---|---|---|
| O. Lifetime and undefined behaviour | ★★★☆☆ | 36, 41 |

## The concept

Prices in ticks, quantities, notional values, sequence numbers and
timestamps in nanoseconds are all integers, and integer bugs in trading
systems are expensive. C++ integers have sharp edges:

| Operation | Signed | Unsigned |
|---|---|---|
| overflow on `+ - *` | **undefined behaviour** | wraps modulo 2^N (defined) |
| `x / 0`, `x % 0` | UB | UB |
| `INT_MIN / -1`, `INT_MIN % -1` | UB (the result does not fit) | n/a |
| `-INT_MIN`, `std::abs(INT_MIN)` | UB | n/a |
| `x << n` with `n >= width` or `n < 0` | UB | UB |
| `x << n` where the result overflows | UB before C++20; defined (modular) since C++20 | defined |
| `x >> n` of a negative `x` | arithmetic shift (defined since C++20) | n/a |
| conversion to a narrower signed type | modular (defined since C++20) | modular |

Signed overflow being UB is not academic: the optimiser assumes it never
happens. `if (a + b < a)` "overflow checks" on signed values are deleted
by GCC at `-O2`, because in a world without overflow the condition is
always false.

**Mixing signed and unsigned** converts the signed side to unsigned:
`-1 < 1u` is `false`. `v.size() - 1` on an empty vector is
`SIZE_MAX`. C++20's `std::cmp_less(a, b)` and friends in `<utility>`
compare mathematically.

### Tools

- `__builtin_add_overflow(a, b, &out)` (GCC/Clang; also `_sub_`, `_mul_`)
  computes the wrapped result and returns whether it overflowed. Compiles
  to an add plus a flag check. (C++26 adds `std::add_sat` etc.)
- Pre-checks against limits: `b > 0 && a > max - b` means `a + b` overflows.
- Do arithmetic in a wider type when one exists (`__int128` on GCC/Clang).
- `std::midpoint(a, b)` (C++20) computes the midpoint without overflow.
- UBSan (`-fsanitize=undefined`) reports signed overflow, bad shifts and
  division problems at run time. The tests here abort on the first report.

### Sequence numbers that wrap

Feeds number packets with 32-bit sequence numbers that **wrap around**.
"Is `a` after `b`?" cannot be `a > b`: just after the wrap, `5` is after
`4294967290`. *Serial number arithmetic* (RFC 1982) answers it: compute the
unsigned difference `a - b` (defined, wraps) and look at it as a signed
32-bit value: positive means `a` is after `b`, as long as the true distance
is less than 2^31.

## Your task

In `integers.hpp` (namespace `idioms`). The tests run under UBSan with
`-fno-sanitize-recover`, so any UB aborts the test binary.

1. `checked_add(a, b)` / `checked_mul(a, b)` for `std::int64_t`: return
   `std::optional<std::int64_t>`, `std::nullopt` on overflow.
2. `saturating_add(a, b)` for `std::int64_t`: clamps to the min/max instead
   of overflowing.
3. `notional(price_ticks, qty)`: `price_ticks * qty` as `std::int64_t`, but
   throw `std::overflow_error` if it does not fit. Inputs are `std::int64_t`.
4. `mid(a, b)`: the midpoint of two `std::int64_t`s rounded towards `a`
   (like `std::midpoint`), with no overflow for any inputs. Do not call
   `std::midpoint`.
5. `safe_abs(x)`: absolute value of a `std::int64_t` as `std::uint64_t`,
   correct for `INT64_MIN`.
6. `seq_after(a, b)`: `true` if 32-bit sequence number `a` comes after `b`
   in serial number arithmetic.
7. `index_before_last(v)`: index of the element before the last of a
   `std::vector<int>`, or `-1` if there is none, without the `size() - 2`
   underflow trap (return type `long`).

<details><summary>Hints</summary>

- `mid`: work in unsigned: `ua = a, ub = b`; if `a <= b` the answer is
  `a + static_cast<std::int64_t>((ub - ua) / 2)`; otherwise
  `a - static_cast<std::int64_t>((ua - ub) / 2)`.
- `safe_abs`: `x < 0 ? 0 - static_cast<std::uint64_t>(x) : static_cast<std::uint64_t>(x)`.
- `seq_after`: `static_cast<std::int32_t>(a - b) > 0`.
</details>
