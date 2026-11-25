# 10 · Iterator invalidation: the rules and the idioms

| Group | Difficulty | Builds on |
|---|---|---|
| D. Iterator invalidation | ★★☆☆☆ | 01–05 |

## The concept

An iterator, pointer or reference into a container is **invalidated** when
the element it refers to moves or dies. Using it afterwards is undefined
behaviour: often it "works" in testing and corrupts memory in production.
ASan catches most of these as `heap-use-after-free`, which is why every test
here runs under it.

### The rules you must know by heart

| Container | Insert | Erase |
|---|---|---|
| `vector`, `string` | if it reallocates (`size() == capacity()` before): **everything**. Otherwise everything at or after the insertion point. | everything at or after the erased element (including `end()`) |
| `deque` | at either end: all **iterators**, but references/pointers stay valid. In the middle: everything. | at either end: only the erased (+ `end()` when erasing the back). In the middle: everything. |
| `list`, `forward_list` | nothing | only the erased element |
| `map`, `set` (and multi-) | nothing | only the erased element |
| `unordered_map`/`set` | if it rehashes: all **iterators**, references/pointers stay valid. Otherwise nothing. | only the erased element |

Things that look harmless but are not:

```cpp
int& best = v.front();
v.push_back(x);            // may reallocate: `best` now dangles

for (auto it = v.begin(); it != v.end(); ++it)
    if (bad(*it)) v.erase(it);   // `it` is invalid after erase; ++it is UB

v.insert(v.end(), v.begin(), v.end());   // UB: the source range is inside *this
```

### The idioms

**Erase–remove.** Move the keepers to the front in one pass, then chop the
tail in one erase. O(n) instead of O(n²):

```cpp
v.erase(std::remove_if(v.begin(), v.end(), pred), v.end());
std::erase_if(v, pred);            // C++20 shorthand for the same thing
```

**Use the returned iterator.** `erase` returns the iterator after the
removed element; `insert` returns an iterator to the inserted element. Keep
going from those:

```cpp
for (auto it = m.begin(); it != m.end(); )
    if (expired(*it)) it = m.erase(it); else ++it;
```

**Indices survive reallocation; iterators do not.** When you must grow a
vector while walking it, walk by index, or `reserve` first so no
reallocation happens (then only the "after the insertion point" rule
applies).

## Your task

In `invalidation.hpp` (namespace `idioms`). The tests use ASan to catch
any use of an invalidated iterator, so a solution that "passes" by luck
will still fail.

1. `remove_cancelled(orders)`: remove every order with `cancelled == true`,
   keeping the relative order of the rest, in O(n). Use erase–remove (no
   `std::erase_if`, practise the real idiom once).
2. `duplicate_evens(v)`: in place, insert a copy of each even number right
   after it: `{1,2,3,4}` becomes `{1,2,2,3,4,4}`. Iterate with iterators
   (not indices) and do **not** call `reserve`: use what `insert` returns.
3. `expire(book, now)`: `book` is a `std::map<int, Quote>` keyed by id.
   Erase every quote with `expires_at <= now` in one pass and return how
   many were erased.
4. `append_self(v)`: make `v` contain its contents twice
   (`{a,b}` becomes `{a,b,a,b}`), without the UB self-insert above and
   without copying the vector into a temporary.
5. `best_then_grow(v, extra)`: returns the maximum element of `v` *after*
   appending all of `extra` to `v`. The stub shows the obvious buggy version
   in a comment (it keeps a reference across `push_back`); write a correct one.
   `v` and `extra` are non-empty.

<details><summary>Hints</summary>

- `duplicate_evens`: `it = v.insert(it, *it);` points at the new copy;
  the original is now at `it + 1`, so step by 2. Careful: `insert(it, *it)`
  passes a reference into the vector itself; the standard requires this to
  work for single-element insert, so it is fine.
- `append_self`: `reserve(2 * n)` first, then a copy of the first `n`
  elements with `std::copy_n` into a `back_inserter` is safe: no
  reallocation, and you only read elements that already existed.
</details>
