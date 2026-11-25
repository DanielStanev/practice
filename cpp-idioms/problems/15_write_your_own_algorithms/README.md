# 15 · Writing generic algorithms the way the standard library does

| Group | Difficulty | Builds on |
|---|---|---|
| E. STL algorithms | ★★★☆☆ | 01, 02, 13 |

## The concept

Using algorithms well comes from knowing how they work inside. Writing a
few yourself, with the same contracts as the standard ones, teaches the
details that interviews probe:

- **Minimal iterator requirements.** `std::remove_if` only needs *forward*
  iterators, `std::lower_bound` too (it uses `std::advance` and
  `std::distance`, which are O(n) on a list but still only O(log n)
  *comparisons*). Asking for random access when forward suffices makes your
  algorithm needlessly unusable on lists.
- **Moves, not copies.** `remove_if` and `unique` shift survivors by
  *move-assignment* (`*dest = std::move(*src)`), so they work with
  move-only types like `std::unique_ptr`. The elements left in the tail are
  valid but unspecified (moved-from).
- **Self-move hazard.** `*it = std::move(*it)` on the same object is
  allowed to leave it empty for some types. Skip the move while the read
  and write positions coincide.
- **Return values carry information.** `remove_if` returns the new logical
  end; `rotate` returns where the old first element ended up; `merge`
  returns the output end. Callers need these.
- **Stability.** `merge` must take from the *first* range when elements are
  equal, otherwise it is not stable.

### `rotate` on forward iterators

The forward-iterator `std::rotate` is a nice puzzle: swap the two blocks
`[first, middle)` and `[middle, last)` using only `++` and `std::iter_swap`.
The classic algorithm (from Stepanov's "Elements of Programming"):

```
next = middle
while first != next:
    swap(*first++, *next++)
    if next == last:      next = middle      // second block exhausted, restart it
    else if first == middle: middle = next   // first block exhausted
```

## Your task

Implement in `my_algorithms.hpp` (namespace `idioms`), using only the
iterator operations each signature's concept allows (no calls to the
`std::` algorithm of the same name, of course):

1. `my_remove_if(first, last, pred)` — forward iterators, moves, returns new end.
2. `my_unique(first, last)` — forward iterators, keeps the first of each run of equal elements.
3. `my_lower_bound(first, last, value, comp = std::less<>{})` — forward
   iterators, O(log n) comparisons. Use `std::distance`/`std::advance`.
4. `my_rotate(first, middle, last)` — forward iterators, returns the new
   position of `*first`. (`first == middle` returns `last`; `middle == last`
   returns `first`.)
5. `my_merge(f1, l1, f2, l2, out, comp = std::less<>{})` — input iterators,
   output iterator, stable, returns the output end. Elements are *copied*.

The tests run each on `std::forward_list`, on move-only element types, and
count comparisons.

<details><summary>Hints</summary>

- `my_remove_if`: find the first element to remove; everything before it
  stays put. Then for each later element that is kept,
  `*dest++ = std::move(*it)`.
- `my_lower_bound`: keep `count = distance(first, last)`; probe the middle
  with `std::next(first, count / 2)`; either discard the left half plus the
  probe, or the right half.
- `my_rotate`'s return value: it is where `first` is the first time `next`
  reaches `last` inside the loop. Remember it then.
</details>
