# 07 · Arena allocator and object pool

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★☆☆ | 02 (raw storage), 03 (object vs. storage lifetime) | bump allocation, alignment arithmetic, intrusive free lists, the Allocator named requirement, keeping the heap off the hot path |

## Why this matters

`malloc` is fast on average and slow in the tail: it can take locks, touch
cold metadata, or call into the kernel. Trading systems therefore allocate
everything at startup and recycle it. The two workhorse patterns are the
**arena** (bump a pointer, free everything at once: great for per-message
scratch space) and the **pool** (fixed-size slots on a free list: great for
orders that come and go). Problem 18's matching engine and problem 20 use a
pool for their orders.

## Task

Implement `hft::Arena`, `hft::ArenaAllocator<T>` and `hft::ObjectPool<T>` in
`arena_and_pool.hpp`.

## Requirements

### Arena
1. Allocates its whole buffer once in the constructor and frees it in the destructor.
2. `allocate(bytes, align)` returns properly aligned memory or `nullptr` when
   full. A failed allocation leaves `used()` unchanged.
3. `create<T>(args...)` constructs a `T` in the arena. Since the arena never
   runs destructors, constrain it to trivially destructible types.
4. `reset()` makes the whole buffer available again. `owns(p)` checks the range.

### ArenaAllocator
5. Satisfies the standard *Allocator* requirements well enough for
   `std::vector<T, ArenaAllocator<T>>`: `value_type`, `allocate`,
   `deallocate` (no-op), a rebinding converting constructor, and `==`.
6. Throws `std::bad_alloc` when the arena is exhausted.

### ObjectPool
7. Allocates `capacity` slots up front. `acquire(args...)` constructs a `T` in
   a free slot (or returns `nullptr` if none); `release(p)` destroys it and
   frees the slot. Both are O(1) and perform **no heap allocation** (the test
   counts calls to `operator new`).
8. The free list is *intrusive*: a free slot's own bytes store the pointer to
   the next free slot. No side vector of indices.
9. LIFO reuse: `release(p)` followed by `acquire()` returns `p` again.
10. Slots respect `alignof(T)` (including `alignas(64)` types).
11. If `T`'s constructor throws, the slot is returned to the pool.

## Running

```bash
./run_tests.sh          # correctness, ASan+UBSan
./run_tests.sh bench    # pool vs new/delete vs arena, -O3
```

<details>
<summary>Hints</summary>

- Aligning up: `(addr + align - 1) & ~(align - 1)` for power-of-two `align`.
  Do it on `std::uintptr_t`, not on the offset, because the base address
  itself matters.
- A slot can be `union Slot { Slot* next; alignas(T) std::byte storage[sizeof(T)]; };`.
- Allocate over-aligned storage with
  `::operator new(bytes, std::align_val_t{alignof(Slot)})` and free it with
  the matching `::operator delete(p, std::align_val_t{...})`.
</details>

## Going further

- Make the arena grow by chaining blocks when it runs out instead of failing.
- Use `std::pmr::monotonic_buffer_resource` and `std::pmr::unsynchronized_pool_resource`
  and compare them with yours in the benchmark.
- Add a debug mode that fills released slots with `0xDD` and checks for
  double release.
- Pre-fault the pool memory (touch every page, or `mlock`) and measure the
  first-run latency difference.

## Reading

- cppreference: [Allocator requirements](https://en.cppreference.com/w/cpp/named_req/Allocator),
  [`std::pmr`](https://en.cppreference.com/w/cpp/memory/memory_resource)
- Andrei Alexandrescu, "std::allocator Is to Allocation what std::vector Is to Vexation" (CppCon 2015)
- John Lakos, "Local ('Arena') Memory Allocators" (CppCon 2017)
- Ulrich Drepper, "What Every Programmer Should Know About Memory", sections 3 and 6
