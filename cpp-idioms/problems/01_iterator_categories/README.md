# 01 · Iterator categories and tag dispatch

| Group | Difficulty | Builds on |
|---|---|---|
| A. Iterator fundamentals | ★☆☆☆☆ | nothing |

## The concept

An iterator is a generalised pointer: something you can dereference (`*it`)
and step (`++it`). The STL separates *algorithms* from *containers* by having
every algorithm talk only to iterators. Since not every iterator can do
everything a pointer can, iterators are sorted into **categories**, each a
superset of the one before:

| Category | Adds | Example |
|---|---|---|
| input | read once, single pass (`*it`, `++it`) | `std::istream_iterator` |
| forward | multi-pass: copies can be stepped independently | `std::forward_list::iterator` |
| bidirectional | `--it` | `std::list`, `std::map`, `std::set` |
| random access | `it + n`, `it - n`, `it2 - it1`, `it[n]`, `<` in O(1) | `std::deque` |
| contiguous (C++20) | elements are adjacent in memory | `std::vector`, `std::array`, `T*` |

(Output iterators, like `std::back_insert_iterator`, are a separate
write-only family; see problem 03.)

The category is an *efficiency contract*. `std::distance(first, last)` is
O(1) for random access iterators (`last - first`) but must walk element by
element for a `std::list`. The same algorithm picks a different
implementation depending on what the iterator can do.

### How the library finds out the category

**C++98 style: `std::iterator_traits` + tag dispatch.** Every iterator
publishes five member types (or `iterator_traits` is specialised for it):

```cpp
template <class It>
struct iterator_traits {
    using iterator_category = typename It::iterator_category; // a tag type
    using value_type        = typename It::value_type;
    using difference_type   = typename It::difference_type;
    using pointer           = typename It::pointer;
    using reference         = typename It::reference;
};
// ...plus a partial specialisation for raw pointers T*.
```

The tags form an inheritance chain:

```cpp
struct input_iterator_tag {};
struct forward_iterator_tag       : input_iterator_tag {};
struct bidirectional_iterator_tag : forward_iterator_tag {};
struct random_access_iterator_tag : bidirectional_iterator_tag {};
struct contiguous_iterator_tag    : random_access_iterator_tag {}; // C++20
```

*Tag dispatch* passes an object of the tag type to an overload set. Overload
resolution picks the most derived match, so a contiguous iterator falls back
to the random-access overload automatically:

```cpp
namespace detail {
template <class It>
auto count_steps(It f, It l, std::input_iterator_tag) { /* loop */ }
template <class It>
auto count_steps(It f, It l, std::random_access_iterator_tag) { return l - f; }
}
template <class It>
auto count_steps(It f, It l) {
    using Cat = typename std::iterator_traits<It>::iterator_category;
    return detail::count_steps(f, l, Cat{});
}
```

**C++20 style: concepts.** `<iterator>` now has `std::input_iterator`,
`std::forward_iterator`, `std::bidirectional_iterator`,
`std::random_access_iterator` and `std::contiguous_iterator`. They check the
actual operations (and, for C++20 iterators, `iterator_concept`). With
`if constexpr` you can write the dispatch inline:

```cpp
if constexpr (std::random_access_iterator<It>) return l - f;
else { /* loop */ }
```

Both styles are everywhere in real code bases, so you will practise both.

## Your task

Implement in `iterator_categories.hpp` (namespace `idioms`):

1. `my_distance(first, last)` using **tag dispatch** on
   `std::iterator_traits<It>::iterator_category` (no `if constexpr`, no
   C++20 iterator concepts). O(1) for random access, a loop otherwise.
2. `my_advance(it, n)` with the same tag-dispatch technique:
   * random access: one `it += n`;
   * bidirectional: loop with `++` or `--` depending on the sign of `n`;
   * input/forward: loop with `++`; a negative `n` throws
     `std::invalid_argument` (the standard calls it UB; we make it loud).
3. `category_name<It>()` returning `"contiguous"`, `"random_access"`,
   `"bidirectional"`, `"forward"`, `"input"` or `"output"`, using the
   **C++20 concepts** and `if constexpr`. Check the most refined concept first.

The tests use a probe iterator that counts how many times `++`, `--` and
`+=` are called, so an O(n) walk on a random access iterator will fail.

## Running the tests

```bash
./run_tests.sh              # ASan + UBSan
./run_tests.sh asan advance # only tests whose name contains "advance"
```

<details><summary>Hints</summary>

- Name the dispatch overloads in a `detail` namespace and take the tag by
  value as the last parameter.
- You only need two `my_distance` overloads: `input_iterator_tag` and
  `random_access_iterator_tag`. Inheritance between tags does the rest.
- `std::output_iterator<It, T>` needs a value type: test
  `!std::input_iterator<It>` last and return `"output"`.
</details>

## Going further

- Read the libstdc++ implementation of `std::advance` in
  `bits/stl_iterator_base_funcs.h`. Spot the tag dispatch.
- Why is `contiguous_iterator_tag` only usable through `iterator_concept`
  and not `iterator_category`? (Hint: backwards compatibility with code that
  compared tags with `std::is_same`.)
