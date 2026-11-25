# 08 · Single-pass input iterators and generators

| Group | Difficulty | Builds on |
|---|---|---|
| C. The C++20 iterator model | ★★☆☆☆ | 04, 07 |

## The concept

Some sequences can only be read once: a socket, a file, a stream of market
data, a random number generator. Their iterators are **input iterators**.
Two copies of an input iterator do not iterate independently; advancing one
may invalidate the other. That is why `std::forward_iterator` requires the
"multi-pass" guarantee and `std::input_iterator` does not.

C++20 made input iterators much easier to write:

- They may be **move-only** (no copy constructor needed).
- `operator++(int)` may return `void`.
- They do not need `operator==` against each other, only against a sentinel.
  `std::default_sentinel_t` (value `std::default_sentinel`) is the
  standard "I know when I'm done" sentinel.
- They need `using value_type`, `using difference_type`, and
  `using iterator_concept = std::input_iterator_tag;`.

The usual shape: the **range object owns the state** (the stream, the
current element) and the iterator is just a pointer back to it.

```cpp
class Numbers {                       // reads ints from a stream, one pass
    std::istream* in_;
    int current_;
    bool done_ = false;
public:
    class iterator {
        Numbers* parent_;
    public:
        using value_type = int;
        using difference_type = std::ptrdiff_t;
        using iterator_concept = std::input_iterator_tag;
        const int& operator*() const { return parent_->current_; }
        iterator& operator++() { parent_->next(); return *this; }
        void operator++(int) { ++*this; }
        bool operator==(std::default_sentinel_t) const { return parent_->done_; }
    };
    iterator begin() { next(); return iterator{this}; }   // reads the first element
    std::default_sentinel_t end() const { return {}; }
};
```

Input ranges compose with views (`filter`, `transform`, `take`), which pull
one element at a time, so a pipeline over a 10 GB log file uses O(1) memory.

Be careful with views that need to look at an element twice: `filter`
calls `*it` once to test and once more when you dereference, so `operator*`
must be cheap and must not advance.

## Your task

In `input_iterators.hpp` (namespace `idioms`):

1. `Lines`: constructed from `std::istream&`. Iterating yields each line as
   `const std::string&` (without the newline). `std::ranges::input_range<Lines>`
   holds, `std::ranges::forward_range<Lines>` does not. `Lines` itself is not
   copyable.
2. `Generator<F>`: wraps a callable `F` returning `std::optional<T>`. Each
   step calls `F` once; iteration ends at the first `std::nullopt`. The
   callable must be called **exactly once per element plus once for the
   final `nullopt`**, no matter how the range is consumed, and not at all
   if the range is never iterated. `make_generator(f)` deduces `F`.
3. `count_errors(std::istream&)`: the number of lines that start with
   `"ERROR"`, using `Lines` with `std::ranges::count_if`.

<details><summary>Hints</summary>

- `Generator` must store the current `std::optional<T>`; `begin()` calls
  `F` for the first time. `T` is
  `typename std::invoke_result_t<F&>::value_type`.
- Do not call `F` in `operator*` or in `operator==`; only in `++` and
  `begin()`.
</details>
