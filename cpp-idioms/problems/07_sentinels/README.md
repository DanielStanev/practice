# 07 · Sentinels: when `end()` is not an iterator

| Group | Difficulty | Builds on |
|---|---|---|
| C. The C++20 iterator model | ★★☆☆☆ | 04 |

## The concept

Before C++20 a range was a *pair of iterators of the same type*. That
forces silly work: to iterate a C string with `std::find` you first call
`strlen` (a full pass) just to build an end pointer. C++20 ranges relax
this: `end()` may return a **sentinel**, any type that can be compared with
the iterator.

```cpp
struct NullSentinel {
    friend bool operator==(const char* p, NullSentinel) { return *p == '\0'; }
};
struct CStr {
    const char* s;
    const char* begin() const { return s; }
    NullSentinel end() const { return {}; }
};

for (char c : CStr{"hello"}) { ... }                      // range-for is fine with it
auto it = std::ranges::find(CStr{"a,b"}, ',');            // single pass, no strlen
```

C++20 generates `!=` and the reversed `sentinel == iterator` from the one
`operator==`. The relevant concepts:

- `std::sentinel_for<S, I>`: `S` is semiregular and `i == s` works.
- `std::ranges::range<R>`: `ranges::begin(r)` and `ranges::end(r)` exist and
  the end is a sentinel for the begin.
- `std::ranges::common_range<R>`: begin and end have the **same** type.
  Legacy algorithms (`std::find`, `std::accumulate`, a vector's
  iterator-pair constructor) need this. `std::views::common` adapts a
  non-common range using `std::common_iterator`.

Sentinels can also carry **state** and a **predicate**: "stop at the first
`'|'`", "stop after n elements" (`std::counted_iterator` +
`std::default_sentinel`), "stop when the parser says the message ends". This
is the natural way to describe binary or text protocol messages whose
length you only discover while reading them.

`std::ranges::subrange(it, sentinel)` turns any iterator/sentinel pair into
a range object you can return and pipe into views.

## Your task

In `sentinels.hpp` (namespace `idioms`):

1. `NullSentinel` and `CStr` as above. `CStr` must satisfy
   `std::ranges::range` and **not** `std::ranges::common_range`.
2. `cstr_length(const char*)`: computed with `std::ranges::distance` over
   `CStr` (no `strlen`, no loops).
3. `UntilSentinel<Pred>`: a sentinel for any iterator whose `==` is true when
   `pred(*it)` is true. Then `until(first, pred)` returns a
   `std::ranges::subrange` from `first` to the first element satisfying
   `pred` (the element itself is excluded). The caller guarantees such an
   element exists. Must work with any input iterator, including
   `const char*` and `std::istreambuf_iterator<char>`.
4. `fix_field(msg, tag)`: FIX messages are `tag=value` fields separated by
   `'\x01'` (SOH). Return the value of `tag` as a `std::string`, or an empty
   string if absent. Example: `"8=FIX.4.2\x01" "35=D\x01" "55=AAPL\x01"`,
   tag `55` gives `"AAPL"`. Implement it by repeatedly applying `until` to
   `CStr`-style iteration; do not compute the message length up front.
5. `sum_until_negative(const std::vector<int>&)`: use `until` with
   `std::views::common` and the *legacy* `std::accumulate` (which needs a
   common range). The vector is guaranteed to contain a negative number.

<details><summary>Hints</summary>

- `UntilSentinel` needs a default constructor to be `semiregular`; give
  `Pred` a default member initialiser or use lambdas without captures (C++20
  capture-less lambdas are default constructible).
- `operator==` for `UntilSentinel` should be a template friend:
  `template <class I> friend bool operator==(const I& it, const UntilSentinel& s)`.
- In `fix_field`, after reading a tag with `until(p, is '=')`, the next
  character is `'='`; step over it and read the value until `'\x01'`.
</details>
