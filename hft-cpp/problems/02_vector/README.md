# 02 · Vector with the strong exception guarantee

| Difficulty | Builds on | New skills |
|---|---|---|
| ★☆☆☆☆ | 01 (RAII, moves, `noexcept`) | raw storage vs. constructed objects, placement new / `std::construct_at`, `std::move_if_noexcept`, exception guarantees, aliasing bugs, over-aligned allocation |

## Why this matters

`std::vector` is the default container in low-latency code because it is
contiguous and cache friendly. Knowing exactly when it allocates, when it
copies versus moves, and what state it is in after an exception is what lets
you reason about latency spikes (a surprise reallocation on the hot path) and
correctness. Writing one also forces you to separate *allocating memory* from
*constructing objects*, which every later problem (pools, arenas, queues) relies on.

## Task

Implement `hft::Vector<T>` in `vector.hpp` without using any standard container.

## Requirements

1. **Storage.** Allocate raw, uninitialised memory with `::operator new(bytes,
   std::align_val_t{alignof(T)})` (or `std::allocator<T>`). `reserve(n)` must
   not construct any `T`; types without a default constructor must work.
2. **Growth.** Capacity grows geometrically (×2 is fine) so `push_back` is
   amortised O(1).
3. **Strong exception guarantee** for `reserve`, `push_back` and
   `emplace_back`: if a constructor throws, the vector is unchanged (same
   size, capacity, buffer, and element values) and nothing leaks.
4. **Move if noexcept.** On reallocation, move elements when `T`'s move
   constructor is `noexcept` (or `T` is not copyable), otherwise copy them.
   This is exactly what `std::move_if_noexcept` encodes.
5. **Aliasing.** `v.push_back(v[0])` must work when it triggers a
   reallocation. ASan will catch you if you free the old buffer first.
6. **Moves are O(1) and `noexcept`**: steal the buffer, never move elements.
7. `at()` throws `std::out_of_range`. `operator[]` is unchecked.
8. `clear()` keeps capacity. `shrink_to_fit()` reduces capacity to `size()`.
9. Every constructed element is destroyed exactly once (tests count them).

## Running the tests

```bash
./run_tests.sh
./run_tests.sh asan strong_guarantee
```

<details>
<summary>Hints</summary>

- `<memory>` has the building blocks: `std::uninitialized_move_n`,
  `std::uninitialized_copy_n`, `std::uninitialized_value_construct_n`,
  `std::destroy_n`, `std::construct_at`. The `uninitialized_*` algorithms
  already destroy what they built if a constructor throws.
- For `emplace_back` on a full vector: allocate the new buffer, construct
  the *new* element there first, then relocate the old elements, then free
  the old buffer. That ordering fixes the aliasing case for free.
- Copy-and-swap gives you a strongly exception-safe copy assignment in three lines.
</details>

## Going further

- Add `insert(pos, value)` with the same guarantees. It is harder than it looks.
- Benchmark `push_back` of 10M ints against `std::vector` with and without `reserve`.
- Read about "trivially relocatable" types (P1144, `folly::fbvector`) and why
  relocating with `memcpy` is a big win for types like `std::string`.

## Reading

- Herb Sutter, *Exceptional C++*, Items 8–17 (exception safety)
- cppreference: [`std::move_if_noexcept`](https://en.cppreference.com/w/cpp/utility/move_if_noexcept),
  [uninitialized memory algorithms](https://en.cppreference.com/w/cpp/memory#Uninitialized_memory_algorithms)
- Andrei Alexandrescu, CppCon 2014 "Optimization Tips" (on growth factors)
