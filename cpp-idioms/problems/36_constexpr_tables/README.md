# 36 · `constexpr` and `consteval`: computing tables at compile time

| Group | Difficulty | Builds on |
|---|---|---|
| M. Compile-time programming | ★★★☆☆ | 33 |

## The concept

Work done by the compiler is work your hot path never does. Lookup tables
(CRC tables, powers of ten for price scaling, character classification for
a parser, bit-reversal tables) are classic candidates: compute them once,
at compile time, and ship them in the binary's read-only data.

| Keyword | Meaning |
|---|---|
| `constexpr` variable | must be initialised by a constant expression; implicitly `const` |
| `constexpr` function | *may* be evaluated at compile time if called with constant arguments in a context that requires a constant; otherwise runs at run time like any function |
| `consteval` function (C++20) | *must* be evaluated at compile time ("immediate function"); a run-time call is a compile error |
| `constinit` variable (C++20) | static/thread storage, initialised at compile time, but **not** const (fixes the static initialisation order fiasco) |
| `if consteval` / `std::is_constant_evaluated()` | branch on whether we are being constant-evaluated |

Since C++20, `constexpr` functions can contain loops, local variables,
`try` blocks, `std::array` manipulation, `std::sort`, virtual calls,
and even transient `new`/`delete` (memory allocated during constant
evaluation must be freed before it ends). They cannot do I/O, call
non-`constexpr` functions, or have undefined behaviour: **UB inside
constant evaluation is a compile error**, which makes `static_assert` a
free UB detector for your arithmetic.

```cpp
constexpr std::array<std::uint64_t, 20> make_pow10() {
    std::array<std::uint64_t, 20> t{};
    std::uint64_t v = 1;
    for (auto& x : t) { x = v; v *= 10; }
    return t;
}
inline constexpr auto kPow10 = make_pow10();   // computed by the compiler
static_assert(kPow10[18] == 1'000'000'000'000'000'000ULL);
```

`throw` inside a `constexpr` function is allowed as long as the throwing
path is not taken during constant evaluation; if it is, compilation fails.
That is a neat way to get compile-time errors with a message:

```cpp
consteval long ticks(std::string_view s) {
    if (s.empty()) throw "empty price";   // compile error if hit at compile time
    ...
}
```

## Your task

In `constexpr_tables.hpp` (namespace `idioms`). Everything must be usable
in constant expressions (the tests evaluate each one inside a template
argument).

1. `kPow10`: `std::array<std::uint64_t, 20>` with `10^0 .. 10^19`.
2. `crc32(std::string_view)`: CRC-32 (IEEE, reflected, polynomial
   `0xEDB88320`, initial value and final XOR `0xFFFFFFFF`) using a 256-entry
   table built by a `constexpr` function and stored in an
   `inline constexpr` variable. Check: `crc32("123456789") == 0xCBF43926`.
3. `consteval long price_to_ticks(std::string_view s, int decimals)`:
   parse a non-negative decimal like `"123.45"` into an integer number of
   ticks with `decimals` places (`"123.45"`, 4 gives `1234500`; `"7"`, 2 gives
   `700`). More fractional digits than `decimals`, or any non-digit other
   than one `'.'`, is an error (throw: a compile error at the call site).
   The stub declares it `constexpr` so the unfinished file compiles; change
   it to `consteval`. Then write `price_to_ticks("1.234", 2)` somewhere and
   read the compiler error (and delete it again: it cannot be a test).
4. `primes_below<N>()`: `std::array` of all primes `< N` with exactly the
   right size. (You need the count as a constant first: compute it with a
   separate `constexpr` function and use it as the array bound.)
5. `kCharClass`: a 256-entry `std::array<std::uint8_t, 256>` with bit 1 for
   digits, bit 2 for ASCII letters, bit 4 for `' '`, `'\t'`, `'\n'`, `'\r'`.

<details><summary>Hints</summary>

- CRC table entry `i`: start with `c = i`; 8 times:
  `c = (c & 1) ? 0xEDB88320 ^ (c >> 1) : c >> 1`.
  Then `crc = table[(crc ^ byte) & 0xFF] ^ (crc >> 8)` per byte.
- `primes_below<N>()`: `std::array<int, count_primes_below(N)>`, where
  `count_primes_below` is a `constexpr` function.
</details>
