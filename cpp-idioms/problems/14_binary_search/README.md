# 14 · The binary search family and heterogeneous lookup

| Group | Difficulty | Builds on |
|---|---|---|
| E. STL algorithms | ★★☆☆☆ | 13 |

## The concept

A sorted `std::vector` searched with binary search beats `std::map` for
read-mostly data (price ladders, symbol tables, reference data) because it
is contiguous. The standard gives you four functions, and the difference
between them is exactly where off-by-one bugs live:

```
 sorted:          10  20  20  20  30
 lower_bound(20)       ^                first element >= 20  (not less than)
 upper_bound(20)                   ^    first element >  20
 equal_range(20)      [^          ^)    both at once
 binary_search(20) -> true              only a bool, rarely what you want
```

`std::partition_point(f, l, pred)` is the general form: given a range where
all `pred == true` elements come before all `pred == false` ones, it finds
the boundary. `lower_bound(x)` is `partition_point(e < x)`.

### Searching by a key that is not the element type

Price levels are structs, but you search by price. The comparator for
`lower_bound` is called as `comp(element, value)`; for `upper_bound` as
`comp(value, element)`. A lambda that only handles one order works for one
of them and fails to compile for the other. Options:

```cpp
// 1. a function object with both overloads
struct ByPrice {
    bool operator()(const Level& l, double p) const { return l.price < p; }
    bool operator()(double p, const Level& l) const { return p < l.price; }
};
// 2. C++20 ranges with a projection: compare prices, not Levels
auto it = std::ranges::lower_bound(levels, 101.5, {}, &Level::price);
```

### Transparent comparators: lookup without constructing a key

`std::map<std::string, int>::find("AAPL")` builds a temporary
`std::string` (possibly heap-allocating) on every lookup. With a
**transparent** comparator (`std::less<>`, which has
`is_transparent`), `find`, `count`, `lower_bound` and `contains` accept any
type comparable with the key, such as `std::string_view`, with no
conversion:

```cpp
std::map<std::string, int, std::less<>> table;
table.find(std::string_view("AAPL"));   // no std::string constructed
```

(`std::unordered_map` got the same in C++20, but needs a transparent hash
as well as a transparent equality.)

## Your task

In `binary_search.hpp` (namespace `idioms`). `levels` is sorted by
ascending price with unique prices. All lookups must be O(log n).

1. `find_level(levels, price)`: pointer to the level with exactly this
   price or `nullptr`. Use `std::lower_bound` with a comparator (not ranges).
2. `qty_between(levels, lo, hi)`: total qty of levels with
   `lo <= price <= hi`. Use `std::ranges::lower_bound` /
   `std::ranges::upper_bound` with a **projection**, then a sum.
3. `count_of(sorted, x)`: occurrences of `x` with `std::equal_range`.
4. `insert_sorted(v, x)`: insert keeping `v` sorted; equal elements go
   **after** existing equal ones (stable). Return the index inserted at.
5. `first_unfilled(orders)`: `orders` is partitioned (all fully filled
   orders first). Return the index of the first unfilled one using
   `std::partition_point`.
6. `SymbolTable`: `add(std::string, int id)` and
   `std::optional<int> find(std::string_view) const`. `find` **must not
   allocate**, even for symbols longer than the small-string buffer (checked
   with an allocation counter).

<details><summary>Hints</summary>

- `find_level`: after `lower_bound`, check `it != end && it->price == price`.
- `insert_sorted`: `upper_bound` gives "after the equal ones".
</details>
