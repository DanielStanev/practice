# 31 · SFINAE and the detection idiom

| Group | Difficulty | Builds on |
|---|---|---|
| K. Templates, SFINAE, concepts | ★★★☆☆ | 01, 19 |

## The concept

**SFINAE**: "Substitution Failure Is Not An Error". When the compiler
substitutes template arguments into a function template's *signature* (its
return type, parameter types, template parameter defaults) and the result
is ill-formed, that template is silently removed from the overload set
instead of causing an error. Before C++20 concepts, this was *the* way to
write "this overload only exists if T has a `.size()`".

```cpp
// Enabled only for integral T:
template <class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string describe(T) { return "integer"; }

template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
std::string describe(T) { return "floating"; }
```

`std::enable_if_t<false, int>` does not exist, so the substitution fails
and that overload disappears.

Only the **immediate context** counts: an error *inside the function body*
(or inside a class template instantiated along the way) is a hard error,
not SFINAE.

### The detection idiom with `std::void_t`

To ask "does `T` have member function `size()`?" as a compile-time bool:

```cpp
template <class T, class = void>
struct has_size : std::false_type {};

template <class T>
struct has_size<T, std::void_t<decltype(std::declval<const T&>().size())>> : std::true_type {};
```

`std::void_t<...>` is `void` if every type in it is well-formed. If
`.size()` exists, the partial specialisation matches (`has_size<T, void>`)
and wins; otherwise substitution fails and the primary template (`false`)
is used. `std::declval<T>()` produces a `T` expression in an unevaluated
context without needing a constructor.

You will meet this pattern in every pre-C++20 code base (and in
libstdc++). Problem 32 replaces it with concepts; knowing both lets you
read old code and write new code.

### Tag dispatch vs `if constexpr` vs SFINAE

- **SFINAE / `enable_if`**: choose between *overloads* (signatures differ).
- **`if constexpr`** (C++17): choose a branch *inside one function*; the
  discarded branch is not instantiated (in a template).
- **Tag dispatch** (problem 01): choose an overload by passing a tag object.

## Your task

In `detection.hpp` (namespace `idioms`), **without C++20 concepts or
`requires`** (this problem is about the C++17 toolbox):

1. Traits `has_size<T>`, `has_reserve<T>`, `has_mapped_type<T>`
   (member type `T::mapped_type`), `is_streamable<T>` (`std::ostream& << const T&`),
   each with an `_v` variable template.
2. `container_size(c)`: `c.size()` if it exists, otherwise
   `std::distance(begin(c), end(c))`. Two overloads selected with
   `std::enable_if_t`.
3. `to_text(x)`: one function using `if constexpr`:
   `std::string` itself → as is; arithmetic → `std::to_string`;
   streamable → via `std::ostringstream`; otherwise `"<?>"`.
4. `append_all(dst, src)`: append `src`'s elements to `dst`, calling
   `dst.reserve(dst.size() + container_size(src))` first if `dst` has
   `reserve`.

<details><summary>Hints</summary>

- `is_streamable`: `std::void_t<decltype(std::declval<std::ostream&>() << std::declval<const T&>())>`.
- `container_size` on a C array: `std::size(arr)` exists, `arr.size()` does
  not. Use `std::begin`/`std::end` in the fallback, they work for arrays.
</details>
