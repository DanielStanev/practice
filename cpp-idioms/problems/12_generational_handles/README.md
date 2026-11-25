# 12 · Handles that never dangle: a generational slot map

| Group | Difficulty | Builds on |
|---|---|---|
| D. Iterator invalidation | ★★★☆☆ | 10, 11 |

## The concept

Problem 11 got stable references by paying for a node container: one heap
allocation per element and pointer chasing on every traversal. Hot paths
usually want the opposite: **contiguous** storage that iterates at memory
bandwidth. But a `std::vector` invalidates every pointer when it grows and
shifts elements when you erase. How do you hand out a reference to an
order that survives both?

**Don't hand out pointers. Hand out handles.** A *slot map* (also called a
generational arena, used in game engines, ECS frameworks, and order
management systems) gives each element a `Handle{index, generation}`:

```
 slots_:    [ {dense=2,gen=1} {free,gen=4} {dense=0,gen=1} {dense=1,gen=2} ]
 values_:   [ C               D            A ]          <- contiguous, iterate this
 owner_:    [ 2               3            0 ]          <- dense index -> slot index
```

- `insert` takes a free slot (or a new one), appends the value to the dense
  `values_` vector, and returns `{slot, slot.generation}`.
- `get(h)` checks `slots_[h.index].generation == h.generation`; if not, the
  handle is **stale** (the element was erased, maybe the slot reused) and you
  get `nullptr` instead of someone else's data. This is the "ABA" problem
  solved with a version counter, the same trick used in lock-free queues.
- `erase(h)` bumps the slot's generation (killing all copies of the handle),
  then fills the hole in `values_` by moving the **last** value into it
  ("swap and pop") and fixing that value's slot to point at its new dense
  index. O(1), no shifting, storage stays dense.
- Free slots are chained into a free list through their `dense` field.

The trade: iteration order is not insertion order (swap-and-pop shuffles),
and `get` costs one extra indirection. In exchange: no allocation per
element, contiguous iteration, O(1) everything, and handles that can be
stored anywhere (even sent to another thread or serialised) and checked
for validity.

## Your task

Implement `idioms::SlotMap<T>` in `slot_map.hpp`:

- `Handle insert(T value)`
- `T* get(Handle h)` / `const T* get(Handle h) const`: `nullptr` if stale or
  never valid (including out-of-range index).
- `bool erase(Handle h)`: false if stale.
- `bool contains(Handle h) const`, `size()`, `empty()`.
- `begin()`/`end()` (const and non-const): iterate the dense values.
  (Contiguous iterators: just return the vector's iterators.)
- `template <class Pred> std::size_t erase_if(Pred pred)`: erase every
  value matching `pred`, return the count. Careful: swap-and-pop moves an
  unvisited element into the current position, so do not advance past it.

Requirements:

- erased slots are reused (the slot vector does not grow forever under
  insert/erase churn), and a reused slot's handles from before the erase
  stay stale;
- a handle stays valid across any number of inserts and erases of other
  elements (even though values move in memory);
- `T` only needs to be move constructible and move assignable.

<details><summary>Hints</summary>

- `struct Slot { std::uint32_t dense_or_next_free; std::uint32_t generation; };`
  and a `free_head_` with a sentinel like `UINT32_MAX` for "none".
- In `erase`, handle the case where the erased element is already the
  last dense element (the "swap" is a self-move: skip it).
- Bump the generation on erase, not on insert, so that `Handle{}`
  (`generation 0`) is never valid if slots start at generation 1.
</details>
