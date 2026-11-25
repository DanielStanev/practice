# 13 · The algorithm toolkit: no raw loops

| Group | Difficulty | Builds on |
|---|---|---|
| E. STL algorithms | ★★☆☆☆ | 01–03 |

## The concept

Sean Parent's rule "no raw loops" is not about style. A named algorithm
states intent (`std::partition` says what it does; a `for` loop with two
indices makes the reader reverse-engineer it), has known complexity, has
been tested for edge cases you forget (empty ranges, one element, all
equal), and is often faster than the loop you would write. Interviewers in
trading firms expect you to reach for the right one immediately.

The ones that come up most, with what they cost:

| Need | Algorithm | Cost |
|---|---|---|
| k smallest/largest, sorted | `std::partial_sort(f, f+k, l)` | O(n log k) |
| the k-th element (median, percentile) | `std::nth_element(f, f+k, l)` | O(n) average |
| keep order, group by predicate | `std::stable_partition` | O(n) with a buffer |
| group by predicate, order irrelevant | `std::partition` | O(n), in place |
| sum of f(x) | `std::transform_reduce(f, l, init, plus, f)` | O(n), parallelisable |
| running totals | `std::partial_sum` / `std::inclusive_scan` | O(n) |
| first pair of neighbours with property | `std::adjacent_find(f, l, pred)` | O(n) |
| bring `[m, l)` before `[f, m)` | `std::rotate(f, m, l)` | O(n) |
| drop adjacent duplicates | `std::unique` + `erase` | O(n) |
| min and max together | `std::minmax_element` | 1.5n comparisons |
| is every / any / no element... | `std::all_of` / `any_of` / `none_of` | O(n), short-circuits |

`nth_element` is the one people forget. After
`std::nth_element(v.begin(), v.begin() + k, v.end())`, `v[k]` is what it
would be if the vector were sorted, everything before it is `<=`, and
everything after is `>=`. Percentiles of a latency sample in O(n).

`std::transform_reduce` with two ranges is a dot product:
`std::transform_reduce(a.begin(), a.end(), b.begin(), 0.0)` computes
`Σ a[i]*b[i]`.

## Your task

In `algorithm_toolkit.hpp` (namespace `idioms`), implement each function
with standard algorithms. No `for`/`while` loops (range-for included).
Lambdas are fine.

1. `top_k(prices, k)`: the `k` highest prices, highest first. `k` may
   exceed the size.
2. `percentile(latencies, p)`: the element at index
   `floor(p * (n - 1))` of the sorted sample (`p` in `[0, 1]`), in O(n).
   Takes the vector by value (you may reorder your copy).
3. `vwap(trades)`: `Σ price·qty / Σ qty` (0 if no quantity). One pass for
   the numerator with `std::transform_reduce`, one for the denominator.
4. `urgent_first(orders)`: reorder so that all `urgent` orders come first,
   both groups keeping their original relative order.
5. `first_gap(prices, max_jump)`: index of the first price `i` such that
   `|prices[i+1] - prices[i]| > max_jump`, or `-1`.
6. `move_to_front(ids, id)`: move the first occurrence of `id` to the front,
   shifting the elements before it back by one; no-op if absent.
7. `running_position(fills)`: signed fill quantities (buy > 0, sell < 0);
   return the position after each fill.
8. `dedupe_sorted(ids)`: remove adjacent duplicates in place.

<details><summary>Hints</summary>

- `top_k`: `std::partial_sort` with `std::greater<>{}`, then `resize`.
- `move_to_front`: find it, then `std::rotate(begin, it, it + 1)`.
- `first_gap`: `std::adjacent_find` returns an iterator to the first of the
  pair.
</details>
