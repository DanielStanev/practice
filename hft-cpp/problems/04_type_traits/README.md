# 04 · Compile-time type traits and constexpr utilities

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★☆☆☆ | 01–03 (templates) | partial specialisation, variadic templates, fold expressions, `constexpr` functions, concepts, compile-time tables, compile-time dispatch |

## Why this matters

Low-latency code moves work from run time to compile time wherever it can:
lookup tables computed by the compiler, buffer sizes derived from a list of
message types, dispatch on a message tag that compiles to a jump table instead
of virtual calls. Problems 05, 06, 12, 15 and 19 all lean on these techniques.
Template metaprogramming questions ("implement `is_same`", "find the index of
a type in a pack") also show up in C++ interviews at trading firms.

## Task

Fill in every placeholder in `type_traits.hpp`. The placeholders compile and
return wrong answers so the tests can run.

## Requirements

1. **Traits**: `is_same`, `remove_cvref`, `conditional`, implemented with
   partial specialisation. Do not forward to the `std::` versions.
2. **TypeList algorithms**: `list_size_v`, `type_at_t`, `index_of_v` (first
   match, `-1` if absent), `contains_v`, `push_front_t`, `max_size_v`.
   You may change `inline constexpr` variable placeholders into class
   templates with specialisations, as long as the names in the tests work.
3. **constexpr utilities**: `is_power_of_two`, `next_power_of_two`,
   `log2_floor`, `pow10_table<N>()`, `count_digits`, `fnv1a`. Tests force
   compile-time evaluation by assigning results to `constexpr` variables, so a
   non-constexpr implementation will fail to compile.
4. **`WireMessage` concept**: trivially copyable, standard layout, has a
   static `kType` convertible to `char`.
5. **`dispatch<List>(tag, f)`**: calls `f(std::type_identity<M>{})` for the
   message type in `List` whose `kType == tag`, returns whether one matched.
   Use a fold expression, not recursion, and keep it `constexpr`.

## Running the tests

```bash
./run_tests.sh
```

<details>
<summary>Hints</summary>

- `is_same`: a primary template with `false` and a partial specialisation
  `is_same<A, A>` with `true`.
- `index_of_v`: you can write an immediately invoked constexpr lambda that
  loops over `constexpr bool matches[] = {is_same_v<T, Ts>...}`.
- `dispatch`: `((tag == Ms::kType ? (f(std::type_identity<Ms>{}), true) : false) || ...)`
  short-circuits on the first match.
- `count_digits` with a `pow10_table<20>()` lookup is the basis of fast
  integer-to-string routines.
</details>

## Going further

- Compile the `dispatch` for a 10-message list with `-O2` on Compiler
  Explorer and look at the assembly. Compare with a `switch`.
- Implement `count_digits` branch-free with `std::countl_zero` (look up
  "Lemire fast digit count").
- Implement `transform_t<List, template<class> class F>` and `filter_t`.

## Reading

- cppreference: [partial specialisation](https://en.cppreference.com/w/cpp/language/partial_specialization),
  [fold expressions](https://en.cppreference.com/w/cpp/language/fold),
  [constraints and concepts](https://en.cppreference.com/w/cpp/language/constraints)
- Walter E. Brown, CppCon 2014 "Modern Template Metaprogramming: A Compendium" (both parts)
- Jason Turner and Ben Deane, CppCon 2017 "constexpr ALL the Things!"
