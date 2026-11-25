# 09 · Output iterators: writing without a container

| Group | Difficulty | Builds on |
|---|---|---|
| C. The C++20 iterator model | ★★☆☆☆ | 03, 08 |

## The concept

Many algorithms produce output through an iterator: `std::copy`,
`std::transform`, `std::set_intersection`, `std::partition_copy`,
`std::merge`, `std::unique_copy`. The output does not have to be a
container. If you write your own **output iterator**, the algorithm can
count, forward, filter or serialise its results *without allocating*,
which is exactly what you want on a hot path.

What an algorithm does to its output iterator `out`:

```cpp
*out = value;   // or *out++ = value;
++out;
```

So the minimal output iterator is:

```cpp
struct Sink {
    using iterator_category = std::output_iterator_tag; // legacy algorithms read this
    using value_type = void;                            // output iterators have no value type
    using difference_type = std::ptrdiff_t;             // required by C++20 (weakly_incrementable)
    using pointer = void;
    using reference = void;
    Sink& operator*() { return *this; }          // *out returns something assignable
    Sink& operator++() { return *this; }         // stepping does nothing
    Sink operator++(int) { return *this; }
    template <class T>
    Sink& operator=(const T& v) { /* consume v */ return *this; }
};
static_assert(std::output_iterator<Sink, int>);
```

The C++20 concept `std::output_iterator` only needs `difference_type`, but
pre-C++20 algorithms such as `std::fill_n` and `std::copy` in libstdc++
still go through `std::iterator_traits<It>::iterator_category`, so without
the tag your iterator works with `std::ranges::copy` and fails to compile
with `std::fill_n`. Provide all five member types.

Returning `*this` from `operator*` and doing the work in `operator=` is the
standard trick (it is how `std::back_insert_iterator` works). The
alternative is returning a small **proxy** object whose `operator=` does the
work.

**Watch out:** algorithms take the output iterator **by value** and return
the final copy. If your iterator keeps state (like a count), read it from
the *returned* iterator, or keep the state outside and hold a pointer to it.

## Your task

In `output_iterators.hpp` (namespace `idioms`):

1. `FunctionOutput<F>`: `*it = v` calls `f(v)`. Must satisfy
   `std::output_iterator<FunctionOutput<F>, T>` for the types `F` accepts.
   `function_output(f)` deduces `F`.
2. `CountingOutput`: counts assignments; `count()` returns the number.
   Assignable from anything.
3. `intersection_size(a, b)`: number of common elements of two sorted
   vectors, using `std::set_intersection` and `CountingOutput`. **Must not
   allocate** (checked with an allocation counter).
4. `split_fills(fills, big, small, threshold)`: copy each fill whose
   `qty >= threshold` into `big`, the rest into `small`, preserving order,
   using a single call to `std::partition_copy` with `std::back_inserter`.
5. `sum_of_squares(v)`: using `std::transform` into a `FunctionOutput`
   that adds to a local `long`. (Yes, `std::transform_reduce` is the real
   answer; this one is for practice.)

<details><summary>Hints</summary>

- `CountingOutput` should be cheap to copy: keep the count as a member and
  have `intersection_size` use the iterator returned by the algorithm.
- `FunctionOutput::operator=` can be a template; make it `const`-correct by
  storing `F` and calling it as non-const (`mutable` lambdas need that).
</details>
