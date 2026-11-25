# 33 · Variadic templates, fold expressions and index sequences

| Group | Difficulty | Builds on |
|---|---|---|
| K. Templates, SFINAE, concepts | ★★★☆☆ | 22, 31, 32 |

## The concept

A **parameter pack** is zero or more template arguments
(`class... Ts`) or function arguments (`Ts... args`). You cannot index it
directly; you *expand* it with `...` after a pattern.

```cpp
template <class... Ts>
void log_all(const Ts&... args) {
    (std::cout << ... << args);                  // binary left fold over <<
    ((std::cout << args << ' '), ...);           // unary right fold over the comma operator
}
sizeof...(Ts)                                    // number of elements
```

**Fold expressions** (C++17), with `op` any binary operator:

| Form | Expands to |
|---|---|
| `(pack op ...)` | `a1 op (a2 op (... op an))` |
| `(... op pack)` | `((a1 op a2) op ...) op an` |
| `(init op ... op pack)` | `((init op a1) op a2) ...` |
| `(pack op ... op init)` | `a1 op (... op (an op init))` |

Unary folds over an **empty** pack are only allowed for `&&` (gives
`true`), `||` (`false`) and `,` (`void()`). Use a binary fold with an init
value when the pack may be empty.

The comma fold is the C++17 way to "loop" over a pack, and it evaluates
left to right. A trick to do something between elements: keep a flag or
index in a lambda captured by reference.

### Index sequences: turning a number into a pack

`std::make_index_sequence<N>` is the type `std::index_sequence<0, 1, ..., N-1>`.
Deduce it in a helper (or a lambda with a template parameter list, C++20)
to get the indices as a pack:

```cpp
template <class Tuple, class F>
void for_each_in_tuple(Tuple&& t, F&& f) {
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (f(std::get<I>(t)), ...);
    }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
}
```

`std::apply(f, tuple)` does the common case (call `f` with all elements).

### Type lists

Packs of *types* are the basis of compile-time programming: message type
registries, variant visitors, serialisation schemas.

```cpp
template <class... Ts> struct TypeList {};
template <class T, class List> struct contains;
template <class T, class... Ts>
struct contains<T, TypeList<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};
```

## Your task

In `variadic.hpp` (namespace `idioms`):

1. `sum(xs...)`: sum of one or more values with a fold.
2. `all_same_v<Ts...>`: true for zero or one type, or all identical.
3. `join_args(sep, args...)`: stream every argument, separated by `sep`
   (none for zero args), into a `std::string`.
4. `for_each_in_tuple(t, f)`: call `f` on each element in order, with
   index sequences (not `std::apply`). Must allow `f` to modify elements of
   a non-const tuple.
5. `transform_tuple(t, f)`: `std::tuple{f(get<0>(t)), f(get<1>(t)), ...}`.
6. `make_table<N>(f)`: `std::array{f(0), f(1), ..., f(N-1)}` with
   `std::make_index_sequence`. Usable in `constexpr`.
7. `TypeList<Ts...>` metafunctions: `list_size_v<L>`, `contains_v<T, L>`,
   `index_of_v<T, L>` (index of the first `T`; does not compile if absent
   — the tests only use present types), `at_t<N, L>`.

<details><summary>Hints</summary>

- `all_same_v`: specialise for zero types, else `(std::is_same_v<First, Rest> && ...)`.
- `join_args`: `bool first = true; ((os << (first ? "" : sep) << args, first = false), ...);`
- `index_of_v`: recursion via partial specialisation, or a `constexpr`
  function that loops over `std::array{std::is_same_v<T, Ts>...}`.
- `at_t`: `std::tuple_element_t<N, std::tuple<Ts...>>` is the cheat;
  writing it by recursion is the exercise.
</details>
