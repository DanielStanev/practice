# 05 · A random access iterator for a ring buffer

| Group | Difficulty | Builds on |
|---|---|---|
| B. Writing iterators | ★★☆☆☆ | 04 |

## The concept

Ring buffers are everywhere in trading systems: market data queues, recent
trade windows, rolling statistics. Their storage is contiguous but the
*logical* sequence wraps around the end of the array, so a raw pointer is
not a valid iterator. You need an iterator that maps a logical position to
a physical slot, and to use `std::sort`, `std::lower_bound` or
`std::nth_element` on it, the iterator must be **random access**.

`std::random_access_iterator<It>` requires everything from bidirectional
(problem 04 plus `--`) and also, all in O(1):

```cpp
it += n;  it -= n;           // return It&
it + n;   n + it;  it - n;   // return It  (note: n + it as well!)
it2 - it1                    // difference_type
it[n]                        // same type as *it
it1 < it2  (and <=, >, >=)   // a total order: use operator<=>
```

plus `iterator_concept`/`iterator_category =
std::random_access_iterator_tag`. Forget `n + it` or `it[n]` and the
concept is false; the compiler error from `std::ranges::sort` will tell you
"constraints not satisfied" and, with GCC, list which requirement failed if
you read the notes carefully. A `static_assert` next to the class is the
fastest feedback loop.

A neat design for the iterator here: store a pointer to the buffer and a
**logical index** (0 = oldest element). Then all arithmetic is integer
arithmetic on the index, comparison compares indices, and `*it` is
`buf->slot(index)`. Only the dereference knows about wrapping.

`operator<=>` can be defaulted or written as
`return a.index_ <=> b.index_;` together with a matching `operator==`.

`std::reverse_iterator` works on top of any bidirectional iterator, so once
your iterator is done, `rbegin()`/`rend()` are one line each.

## Your task

Implement `idioms::RingBuffer<T, N>` in `ring_buffer.hpp`: a fixed-capacity
FIFO with no heap allocation.

- `bool push_back(T)`: false (and no change) if full.
- `void pop_front()`: precondition: not empty.
- `size()`, `empty()`, `full()`, `capacity()` (== N), `front()`, `back()`.
- `operator[](i)`: logical index, 0 is the oldest element.
- `begin/end` (const and non-const), `cbegin/cend`, `rbegin/rend`.
- `iterator` and `const_iterator` (one template, as in problem 04) both
  satisfy `std::random_access_iterator`; `iterator` converts to
  `const_iterator`.

The tests sort, binary search and reverse a buffer whose contents wrap
around the physical end of storage, so any confusion between logical and
physical positions shows up immediately.

Storage hint: `std::array<T, N>` requires `T` to be default constructible,
which is fine for this problem (the tests use `int` and `std::string`).

<details><summary>Hints</summary>

- Keep `head_` (physical index of the oldest element) and `size_`. Physical
  slot of logical `i` is `(head_ + i) % N`.
- Write `+=` first, then express `-=`, `+`, `-`, `++`, `--` with it.
- `difference_type` is signed; the logical index can be `std::ptrdiff_t`
  too, which avoids sign-conversion warnings.
</details>
