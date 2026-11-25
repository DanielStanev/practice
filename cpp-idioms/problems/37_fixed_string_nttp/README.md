# 37 · Strings as template parameters and compile-time containers

| Group | Difficulty | Builds on |
|---|---|---|
| M. Compile-time programming | ★★★★☆ | 33, 36 |

## The concept

### Class types as non-type template parameters (C++20)

Before C++20, a template could be parameterised on integers, enums and
pointers, but not on a string. C++20 allows any **structural** literal
class type as a non-type template parameter (NTTP): all members public, no
`mutable`, and every base and member itself structural. A fixed-size char
array wrapper qualifies:

```cpp
template <std::size_t N>
struct FixedString {
    char data[N]{};                                   // includes the '\0'
    constexpr FixedString(const char (&s)[N]) { std::copy_n(s, N, data); }
    constexpr std::string_view view() const { return {data, N - 1}; }
};
// Deduction guide is implied by the constructor: FixedString("abc") is FixedString<4>.

template <FixedString Name>
struct Field { static constexpr std::string_view name = Name.view(); };

Field<"price"> f;     // the string is part of the type
```

Uses in trading code: compile-time message schemas (`Field<"38", Qty>`),
named metrics without run-time string handling, compile-time hashed symbol
ids, format strings checked at compile time (what `std::format` does).

Two template arguments are the same if their values are equal, so
`Field<"px">` in two translation units is the same type.

### `constexpr` containers

`std::array` is fully `constexpr`; so are most algorithms in C++20
(`std::sort`, `std::find`, `std::lower_bound`...). A fixed-capacity vector
built on `std::array` can be filled at compile time, which lets you build
sorted lookup tables, perfect hash tables, or parse configuration at
compile time and keep only the result.

C++20 `std::vector` and `std::string` are `constexpr` too, but memory
allocated during constant evaluation cannot survive into run time, so they
are only useful *inside* a `constexpr` computation. To return the result,
copy it into a `std::array` whose size you computed first.

## Your task

In `fixed_string.hpp` (namespace `idioms`):

1. `FixedString<N>`: constructor from a string literal, `size()` (without
   the terminator), `view()`, `operator==` (defaulted), and usable as a NTTP.
2. `Field<FixedString Name, class T>`: `static constexpr std::string_view
   name`, a member `T value{}`, and `static constexpr std::uint32_t id`: the
   FNV-1a 32-bit hash of the name (offset basis `2166136261`, prime
   `16777619`).
3. `StaticVector<T, Cap>`: `constexpr` `push_back` (throws
   `std::length_error` when full; during constant evaluation that is a
   compile error, since C++20 does not allow throwing there even if you catch), `size`, `capacity`, `operator[]`,
   `begin`/`end`, `back()`, `pop_back()`.
4. `make_symbol_table(names)`: `consteval`/`constexpr` function that takes a
   `std::array<std::string_view, N>`, sorts it and checks there are no
   duplicates (throw if there are). Then `symbol_index(table, name)`:
   `constexpr` binary search returning the index or `-1`.

<details><summary>Hints</summary>

- FNV-1a: `h = 2166136261; for each byte: h ^= byte; h *= 16777619;` with
  `std::uint32_t` arithmetic (wraps, no UB for unsigned).
- `StaticVector` storage: `std::array<T, Cap> data_{}` and a size.
- `make_symbol_table`: `std::sort(names.begin(), names.end())`, then
  `std::adjacent_find`.
</details>
