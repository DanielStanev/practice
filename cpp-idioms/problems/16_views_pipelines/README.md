# 16 · Ranges and views: lazy pipelines

| Group | Difficulty | Builds on |
|---|---|---|
| F. Ranges | ★★☆☆☆ | 07, 08, 13 |

## The concept

A **view** is a lightweight range that does not own its elements and is
cheap to copy. *Range adaptors* build views from other ranges and compose
with `|`:

```cpp
auto big_aapl = trades
    | std::views::filter([](const Trade& t) { return t.symbol == "AAPL"; })
    | std::views::transform(&Trade::qty)
    | std::views::take(10);
```

Nothing has happened yet. `big_aapl` is a small object describing the
computation; elements are produced one at a time when you iterate it. That
gives you:

- **no intermediate containers** (no allocation between stages),
- **short-circuiting**: `take(10)` stops pulling after 10 elements,
- **composability**: a pipeline is a value you can return and extend.

The adaptors you will use most (C++20):

| Adaptor | Does |
|---|---|
| `views::filter(pred)` | keep elements where `pred` is true |
| `views::transform(f)` | map each element through `f` (member pointers work) |
| `views::take(n)` / `drop(n)` | first n / skip n |
| `views::take_while(p)` / `drop_while(p)` | prefix while `p` / skip prefix while `p` |
| `views::reverse` | needs a bidirectional range |
| `views::keys` / `views::values` / `views::elements<I>` | pick tuple/pair members |
| `views::join` | flatten a range of ranges |
| `views::split(delim)` | split into subranges |
| `views::iota(a, b)` | the integers `[a, b)` |
| `views::common` | make begin/end the same type (for legacy algorithms, problem 07) |
| `views::counted(it, n)` | `n` elements from `it` |

C++20 has no `std::ranges::to` (that is C++23), so to collect a view into
a container use the iterator-pair constructor on a common range, or
`std::ranges::copy(view, std::back_inserter(vec))`.

### Gotchas

- **Views hold references.** A view over a local vector dangles once the
  vector dies. Never return a view of a local.
- `filter`'s `begin()` is computed once and **cached**; a filter view over a
  container you then modify may give stale results. Build views just
  before you use them.
- Lambdas in views run **every time** the element is visited (transform
  is not memoised). `filter | transform` can call the transform function
  more than once per element if something dereferences twice.
- Iterating a `filter_view` requires a non-const view (because of the
  cache): take views by value or `auto&&`, not `const auto&`.

## Your task

In `views_pipelines.hpp` (namespace `idioms`), write each function as a
view pipeline plus at most one collection step. No raw loops except a
range-for to collect results if you like.

1. `symbols_traded(trades, min_qty)`: symbols of trades with
   `qty >= min_qty`, in order, as `std::vector<std::string>`.
2. `split_fields(line, sep)`: split a `std::string_view` on a character
   into `std::vector<std::string_view>` (views into `line`; empty fields
   kept: `"a,,b"` gives `{"a", "", "b"}`).
3. `last_even(v, n)`: the last `n` even numbers of `v`, latest first.
4. `returns(prices)`: simple returns `prices[i]/prices[i-1] - 1` for
   `i = 1..n-1`. Use `views::iota` to generate the indices.
5. `total_position(book)`: sum of the values of a `std::map<std::string, long>`
   using `views::values`.
6. `all_fill_ids(batches)`: flatten `std::vector<std::vector<int>>` with
   `views::join`.
7. `first_after_open(events)`: given `(time, price)` pairs sorted by time,
   skip events before `open_time`, then take the prices while
   `price > 0`. Use `drop_while` and `take_while`.

<details><summary>Hints</summary>

- `split_fields`: each element of `line | std::views::split(sep)` is a
  subrange of `string_view` iterators, which are contiguous, so
  `std::string_view(sub.begin(), sub.end())` works.
- `returns`: `std::views::iota(std::size_t{1}, prices.size())`. Beware of
  `prices.size() == 0`.
</details>
