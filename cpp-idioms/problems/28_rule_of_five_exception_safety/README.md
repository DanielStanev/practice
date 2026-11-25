# 28 · The rule of five with two resources: exception-safe copying

| Group | Difficulty | Builds on |
|---|---|---|
| J. Rule of 0/3/5 | ★★★☆☆ | 20, 25 |

## The concept

**Rule of three (C++98) / five (C++11):** if a class needs a user-written
destructor, copy constructor or copy assignment, it almost certainly needs
all of them (plus the two move operations), because the need comes from
owning a resource that the compiler-generated versions would shallow-copy.

Problem 20 owned *one* buffer. With **two** resources, the order of
operations becomes an exception-safety question:

```cpp
class Columns {
    double* prices_;
    long*   sizes_;
    std::size_t n_;
public:
    Columns(const Columns& o)
        : prices_(new double[o.n_]),     // succeeds
          sizes_(new long[o.n_]),        // throws std::bad_alloc...
          n_(o.n_) { ... }               // ...and prices_ leaks
};
```

If a constructor throws, the destructor of *that* object never runs (the
object never finished being constructed). Only fully constructed
**members** are destroyed. Raw pointers have no destructor, so the first
allocation leaks. Fixes, in increasing elegance:

1. `try`/`catch` in the constructor body, delete and rethrow.
2. Allocate into local `std::unique_ptr`s first, then `release()` them into
   the members once nothing else can throw.
3. Make the members `std::unique_ptr<double[]>`: then they are fully
   constructed members and clean up automatically. (Problem 29 takes this
   to its conclusion.)

### Assignment: the three guarantees

- **basic**: on exception, no leaks and the object is valid (but maybe changed);
- **strong**: on exception, the object is unchanged ("commit or rollback");
- **nothrow**: never throws.

**Copy-and-swap** gives the strong guarantee for free: do all the
throwing work (the copy) into a temporary, then `swap` (which is
`noexcept`) and let the temporary's destructor free the old state.

```cpp
Columns& operator=(const Columns& o) {
    Columns tmp(o);     // may throw: *this untouched
    swap(tmp);          // noexcept
    return *this;       // tmp's destructor frees our old buffers
}
```

The cost: it always allocates, even when the existing buffers are big
enough. High-performance code sometimes deliberately chooses the basic
guarantee and reuses capacity (as `std::vector::operator=` does). Know both
and say which one you chose.

## Your task

Implement `idioms::OrderColumns` in `order_columns.hpp`: a column store
with two raw arrays, `double* prices_` and `long* sizes_` (keep them raw:
this problem is about doing it by hand), allocated with `new[]`.

- `explicit OrderColumns(std::size_t n)`: both arrays value-initialised (zeros).
- `size()`, `price(i)`, `qty(i)` (references, so tests can write).
- Copy constructor: deep copy, **no leak if either allocation throws**.
- Copy assignment: **strong guarantee** (copy-and-swap).
- Move constructor and move assignment: `noexcept`, steal; moved-from has
  `size() == 0` and null arrays.
- `swap` member and free function, `noexcept`.
- Destructor.

The tests replace `operator new[]` with a version that can be told to
throw on the N-th allocation, and run under ASan's leak checker.

<details><summary>Hints</summary>

- Copy constructor with option 2: `auto p = std::make_unique<double[]>(n);
  auto s = std::make_unique<long[]>(n);` then copy, then `prices_ =
  p.release(); sizes_ = s.release();`. Note `make_unique<T[]>` value-initialises.
- Use the member initialiser list for `n_` *after* the arrays are safe,
  or set it in the body.
</details>
