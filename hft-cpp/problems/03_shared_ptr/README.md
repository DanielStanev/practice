# 03 · SharedPtr and WeakPtr

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★☆☆☆ | 01, 02 | control blocks, type erasure via virtual functions, `std::atomic` reference counting, memory ordering basics, object lifetime vs. storage lifetime |

## Why this matters

You will rarely use `shared_ptr` on a hot path, and this problem shows you
why: every copy is an atomic read-modify-write on a shared cache line. But
interviewers love it because a correct implementation touches lifetime
management, atomics, and the classic `weak_ptr::lock()` race. It is also
your first contact with `std::atomic`, which problems 11–14 build on.

## Task

Implement `hft::SharedPtr<T>`, `hft::WeakPtr<T>` and `hft::make_shared<T>()`
in `shared_ptr.hpp`.

## Requirements

1. **Shared ownership.** Copies share one object. The object is destroyed
   exactly once, when the last `SharedPtr` goes away.
2. **Weak references.** `WeakPtr` observes without owning. The object dies
   when the strong count reaches zero; the control block lives until the
   strong *and* weak counts reach zero.
3. **`lock()`** returns a `SharedPtr` that keeps the object alive, or an empty
   one if it has expired. It must be correct when another thread releases the
   last `SharedPtr` at the same moment. (Hint: you cannot just check then
   increment.)
4. **Thread safety.** Copying and destroying *different* `SharedPtr` objects
   that share ownership is safe from many threads, like `std::shared_ptr`.
   Tests run under ThreadSanitizer.
5. **`make_shared` does one allocation** for the object and control block.
   `SharedPtr(new T)` does exactly one more (the control block). The test
   replaces global `operator new` to count.
6. **Moves are `noexcept`** and do not touch the counts. Self-assignment is safe.
7. `SharedPtr(T*)` must not leak `p` if allocating the control block throws.

Not required: aliasing constructor, custom deleters/allocators,
`enable_shared_from_this`, converting `SharedPtr<Derived>` to `SharedPtr<Base>`.

## Running the tests

```bash
./run_tests.sh           # ASan+UBSan, then TSan
./run_tests.sh tsan concurrent
```

<details>
<summary>Hints</summary>

- An abstract control block with two virtual functions, `destroy_object()`
  and `destroy_self()`, covers both allocation strategies. The
  `make_shared` block stores the object inline in
  `alignas(T) unsigned char storage[sizeof(T)]`.
- A common trick: all strong references together hold *one* weak reference.
  Then "free the block" is simply "weak count hit zero".
- Increments can be `memory_order_relaxed`. Decrements need `acq_rel` so the
  thread that destroys the object sees every other thread's writes to it.
- `lock()` is a compare-exchange loop that increments the strong count only
  if it is not zero.
</details>

## Going further

- Add `enable_shared_from_this`.
- Benchmark copying a `SharedPtr` across 1, 2, 4, 8 threads. Explain the
  curve in terms of cache-line ownership (MESI).
- Read how libstdc++ avoids atomics entirely when the program is single
  threaded (`__gthread_active_p`).

## Reading

- cppreference: [`std::shared_ptr`](https://en.cppreference.com/w/cpp/memory/shared_ptr),
  [`std::memory_order`](https://en.cppreference.com/w/cpp/atomic/memory_order)
- Herb Sutter, GotW #89 and #91 (smart pointers and parameters)
- Anthony Williams, *C++ Concurrency in Action* (2nd ed.), chapter 5
