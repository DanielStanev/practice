# 17 · Writing your own view and range adaptor

| Group | Difficulty | Builds on |
|---|---|---|
| F. Ranges | ★★★★☆ | 04, 07, 08, 16 |

## The concept

The standard views cover a lot, but C++20 is missing some obvious ones
(`stride`, `adjacent`/`pairwise`, `chunk`, `zip` all arrived in C++23). In a
code base stuck on C++20 you write them yourself, and doing so is the best
way to understand how views work.

A view is three pieces:

**1. The view class.** It stores the underlying view (`V base_`) plus
parameters, derives from `std::ranges::view_interface<Derived>` (which
supplies `empty()`, `operator bool`, `front()`, `size()` and so on from your
`begin()`/`end()`), and provides `begin()` and `end()`.

```cpp
template <std::ranges::view V>
class MyView : public std::ranges::view_interface<MyView<V>> {
    V base_ = V();
public:
    MyView() requires std::default_initializable<V> = default;
    explicit MyView(V base) : base_(std::move(base)) {}
    auto begin() { ... }      // returns your iterator
    auto end()   { ... }      // returns a sentinel (std::default_sentinel is handy)
};
```

Storing `V` **by value** is right: `V` is itself a view (cheap to copy).
To accept containers, a deduction guide wraps them with `std::views::all`:

```cpp
template <class R>
MyView(R&&) -> MyView<std::views::all_t<R>>;
```

`views::all(lvalue_container)` gives a `ref_view` (a pointer to the
container); `views::all(rvalue_container)` gives an `owning_view` (moves the
container in), so temporaries do not dangle.

**2. The iterator** (problems 04 and 08), usually a nested class storing
an iterator into the base plus whatever state it needs.

**3. The adaptor object**, which makes `range | my_adaptor(args)` work:

```cpp
struct MyAdaptor {
    int arg;
    template <std::ranges::viewable_range R>
    friend auto operator|(R&& r, MyAdaptor a) { return MyView(std::views::all(std::forward<R>(r)), a.arg); }
};
inline MyAdaptor my_adaptor(int arg) { return {arg}; }
```

(C++23 adds `std::ranges::range_adaptor_closure` so your adaptor composes
with standard ones *before* being applied; in C++20 the friend `operator|`
works on the left-hand range, which covers `v | filter(...) | my_adaptor(2)`.)

Finally, `static_assert(std::ranges::view<MyView<...>>)` and
`std::ranges::forward_range<...>` tell you immediately whether you got it
right.

## Your task

In `custom_views.hpp` (namespace `idioms`):

1. `StrideView<V>` and `stride(n)`: every `n`-th element starting with the
   first. `{0..9} | stride(3)` gives `0 3 6 9`. Must not step past the end
   of the base range: use `std::ranges::advance(it, n, end)`.
   It must be a `std::ranges::view` and a `std::ranges::forward_range` when
   `V` is.
2. `PairwiseView<V>` and `pairwise`: each adjacent pair, by value:
   `{1, 2, 4, 7}` gives `(1,2) (2,4) (4,7)`. The element type is
   `std::pair<T, T>` (copies). Empty and one-element ranges give nothing.
   Also a forward view.
3. `spreads(prices)`: using `pairwise`, the absolute difference of every
   adjacent pair, as a vector.

Both views use `std::default_sentinel_t` as their sentinel, so the base
range does not need to be common.

The tests compose your adaptors with standard ones, apply them to
temporaries (owning views), and to `std::forward_list`.

<details><summary>Hints</summary>

- `StrideView::iterator` stores `current_`, `end_` (base sentinel) and `n_`.
  `++` is `std::ranges::advance(current_, n_, end_)`. Comparing with
  `std::default_sentinel` is `current_ == end_`.
- For the iterator to be a forward iterator, `==` between two iterators must
  also exist (compare `current_`), and it must be default constructible.
- `PairwiseView::iterator` stores `current_` and `next_`; it is at the end
  when `next_ == end_`. In `begin()`, if the base is empty, set `next_ = current_`
  (which equals end).
- Give the iterators `using iterator_concept = std::forward_iterator_tag;`
  and `using iterator_category = std::input_iterator_tag;` when `operator*`
  returns by value.
</details>
