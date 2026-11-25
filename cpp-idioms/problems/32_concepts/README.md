# 32 · Concepts: naming requirements and overloading on them

| Group | Difficulty | Builds on |
|---|---|---|
| K. Templates, SFINAE, concepts | ★★★☆☆ | 02, 31 |

## The concept

A **concept** is a named compile-time predicate on types. It replaces the
`enable_if`/`void_t` machinery of problem 31 with something readable,
gives far better error messages, and supports *ordering* overloads by how
constrained they are.

```cpp
template <class T>
concept Price = std::regular<T>                 // copyable, default constructible, ==
             && std::totally_ordered<T>         // < > <= >=
             && requires(T a, T b) {            // a requires-expression
                    { a + b } -> std::same_as<T>;   // compound requirement: expression + result type
                    { a - b } -> std::same_as<T>;
                };
```

Inside a `requires` expression you can write:

| Requirement | Form |
|---|---|
| simple: the expression compiles | `a.flush();` |
| type: the type exists | `typename T::price_type;` |
| compound: compiles, maybe `noexcept`, result satisfies a concept | `{ a.size() } noexcept -> std::convertible_to<std::size_t>;` |
| nested: an extra boolean | `requires sizeof(T) <= 16;` |

Four ways to apply a concept, all equivalent:

```cpp
template <Price P> P twice(P p);
template <class P> requires Price<P> P twice(P p);
template <class P> P twice(P p) requires Price<P>;
auto twice(Price auto p);                       // abbreviated function template
```

### Overloading and subsumption

When several constrained overloads are viable, the compiler picks the
**most constrained** one, but only if their constraints are built from the
*same* concepts so it can see that one implies the other. `std::signed_integral<T>`
is defined as `std::integral<T> && std::is_signed_v<T>`, so it subsumes
`std::integral<T>` and wins for `int`. Two concepts that are logically
related but written separately (say, your own `Numeric` defined with
`std::is_arithmetic_v` and `std::integral`) are *not* comparable: the
call is ambiguous. Build concepts from other concepts to get ordering.

### Constraining class templates

`template <Price P> class Level { ... };` makes `Level<std::string>`
ill-formed, and `requires { typename Level<std::string>; }` is `false`.

## Your task

In `concepts.hpp` (namespace `idioms`):

1. `Price<T>` as above, plus: `T{}` must compare equal to `T{} + T{}` is
   **not** checkable at compile time, so don't try; just the syntactic
   requirements. (Semantics are documented, not checked: that is the
   general rule for concepts.)
2. `Book<B>`: `B::price_type` exists and is a `Price`; on a `const B`,
   `best_bid()` and `best_ask()` return something convertible to
   `std::optional<typename B::price_type>`; on a `B`,
   `add(Side, price_type, long)` compiles.
3. `spread(const Book auto& b)`: `best_ask - best_bid`, or `std::nullopt`
   if either side is empty.
4. `describe(x)` overloads: `"signed integral"` for `std::signed_integral`,
   `"integral"` for other `std::integral`, `"floating"` for
   `std::floating_point`, `"price"` for any other `Price`, and `"other"`
   for everything else. No `if constexpr`: overloads only, ordered by
   subsumption. (For `"price"` vs the numeric overloads, think about
   which concept has to be part of which.)
5. `Level<P>`: a class template constrained on `Price`, with members
   `P price; long qty;`.
6. `total(r)`: sum of a range whose value type is a `Price`, starting
   from `T{}`.

<details><summary>Hints</summary>

- Making `describe(int)` choose `"signed integral"` over `"price"`: `int`
  is a `Price` too. Constrain the numeric overloads with
  `Price<T> && std::signed_integral<T>` etc., so they subsume the `Price`
  one. Then `std::string` (not a Price) falls to `"other"`.
- `total`: `template <std::ranges::input_range R> requires Price<std::ranges::range_value_t<R>>`.
</details>
