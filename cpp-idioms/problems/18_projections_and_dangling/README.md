# 18 · Projections, borrowed ranges and `std::ranges::dangling`

| Group | Difficulty | Builds on |
|---|---|---|
| F. Ranges | ★★★☆☆ | 13, 14, 16 |

## The concept

### Projections

Every `std::ranges::` algorithm takes an optional **projection**: a callable
applied to each element *before* the comparison or predicate. Member
pointers work as projections (they are invoked with `std::invoke`):

```cpp
std::ranges::sort(orders, {}, &Order::price);                 // sort by price
auto best = std::ranges::max_element(orders, {}, &Order::price);
auto it   = std::ranges::find(orders, 42, &Order::id);        // find by id
std::ranges::sort(orders, std::greater{}, &Order::price);     // descending
```

The `{}` is the default comparator (`std::ranges::less`). This replaces the
pile of hand-written lambdas like `[](auto& a, auto& b) { return a.price <
b.price; }`, and the comparison and the key can no longer drift apart.

For multi-key ordering, project to a `std::tuple` of references and let
tuple's lexicographic `<` do the work:

```cpp
std::ranges::sort(orders, {}, [](const Order& o) { return std::tie(o.price, o.time); });
```

Careful with mixed directions ("price descending, then time ascending"):
negate a numeric key or write the comparator explicitly.

### Dangling protection

```cpp
std::vector<int> load();
auto it = std::ranges::find(load(), 7);   // the vector dies at the semicolon
```

With `std::find(load().begin(), ...)` this compiles and dangles. The ranges
version returns `std::ranges::dangling` instead of an iterator when you pass
an rvalue range that does not promise its iterators outlive it, so
`*it` is a compile error. The promise is called being a **borrowed range**:
`std::string_view`, `std::span`, `ref_view`, `iota_view` and similar
non-owning types are borrowed; containers are not.

Your own non-owning range type opts in with:

```cpp
template <>
inline constexpr bool std::ranges::enable_borrowed_range<MySpan> = true;
```

(Only do this if iterators really do not point into the range object
itself.)

## Your task

In `projections.hpp` (namespace `idioms`):

1. `sort_book(orders)`: price **descending**, then time ascending, then id
   ascending, with one `std::ranges::sort` call and a projection.
2. `largest_order(orders)`: pointer to the order with the greatest `qty`
   (first one if tied) or `nullptr`, with `std::ranges::max_element` and a
   member-pointer projection. Read the docs: which one does it return on ties?
3. `find_by_id(orders, id)`: `std::ranges::find` with a projection; pointer
   or `nullptr`.
4. `qty_extremes(orders)`: `{min_qty, max_qty}` as a `std::pair<long, long>`
   using `std::ranges::minmax` over a `transform` view. Precondition:
   non-empty.
5. `IntSpan`: a non-owning `{const int* data; std::size_t size;}` range with
   `begin()`/`end()`. Make it a **borrowed range**, so that
   `std::ranges::find(IntSpan{...}, x)` on a temporary `IntSpan` returns a
   real iterator.
6. `index_of_first_negative(IntSpan)`: using that property directly on the
   by-value parameter, return the index or `-1`.

The tests check the dangling/borrowed behaviour at compile time with
`std::is_same_v<..., std::ranges::dangling>`.

<details><summary>Hints</summary>

- For price descending: project to `std::tuple(-o.price, o.time, o.id)`
  (by value, so no `tie`), or use a custom comparator with a tie projection.
- `std::ranges::max_element` returns the **first** largest element (unlike
  `std::minmax_element`'s max, which returns the *last*).
</details>
