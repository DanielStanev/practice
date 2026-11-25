# 06 · SmallFunction: type erasure with a small buffer

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★☆☆ | 01–04 (moves, placement new, constraints) | type erasure, hand-rolled vtables, small-buffer optimisation, `std::launder`, `std::invoke` |

## Why this matters

Callbacks are everywhere in trading code: market data handlers, timer
callbacks, order-ack continuations. `std::function` may allocate when you
store a lambda, and an allocation on the hot path is a latency spike.
Production systems use fixed-buffer function wrappers (Folly's `Function`,
Seastar's `noncopyable_function`, C++23's `std::move_only_function`). Building
one teaches type erasure, the technique behind `std::any`, `std::function`
and virtual dispatch itself.

## Task

Implement `hft::SmallFunction<R(Args...), BufferSize>` in `small_function.hpp`.

## Requirements

1. Stores any callable (lambda, functor, function pointer) invocable as
   `R(Args...)`. Construction from anything else does not compile (use a
   `requires` clause with `std::is_invocable_r_v`).
2. **Inline storage.** A callable is stored inside the object, with no heap
   allocation, if `sizeof(F) <= BufferSize`, `alignof(F) <=
   alignof(std::max_align_t)` and `F` is nothrow move constructible.
   Otherwise it is heap allocated. `is_inline()` reports which.
3. **Move-only**, like `std::move_only_function`. Moves are `noexcept`. Moving
   a heap-stored callable steals the pointer and does not allocate.
4. **Size.** At most `BufferSize + 2 * sizeof(void*)`. Do not store several
   function pointers per object; point at one static table per callable type.
5. Calling an empty `SmallFunction` throws `std::bad_function_call`.
6. Every stored callable is destroyed exactly once (on reset, assignment, or
   destruction).
7. Non-const `operator()` (mutable lambdas work). Arguments are forwarded, so
   move-only and reference parameters work.

## Running the tests

```bash
./run_tests.sh
```

<details>
<summary>Hints</summary>

- Make a struct of function pointers: `invoke(void*, Args&&...)`,
  `move(void* dst, void* src)`, `destroy(void*)`. For each callable type `F`
  define a `static constexpr` instance of it (a variable template works
  well), filled with captureless lambdas. Store a pointer to it.
- For heap storage, put the `F*` itself in the buffer. Then the same three
  operations work for both cases.
- After placement new into a byte buffer, get the object back with
  `std::launder(reinterpret_cast<F*>(buf_))`.
- Why must inline callables be nothrow movable? Think about what the
  `noexcept` move constructor of `SmallFunction` has to do.
</details>

## Going further

- Add a copyable variant and compare the vtable you need.
- Benchmark calling through `SmallFunction`, `std::function` and a virtual
  interface in a loop. Then benchmark *constructing* each from a lambda
  capturing 40 bytes.
- Read the `std::move_only_function` proposal (P0288) for the design trade-offs.

## Reading

- Sean Parent, "Inheritance Is the Base Class of Evil" (GoingNative 2013)
- Arthur O'Dwyer, "Back to Basics: Type Erasure" (CppCon 2019)
- cppreference: [`std::launder`](https://en.cppreference.com/w/cpp/utility/launder),
  [`std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function)
