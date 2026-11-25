# 38 · Lambdas: what a capture really is

| Group | Difficulty | Builds on |
|---|---|---|
| N. Lambdas | ★★☆☆☆ | 20, 22 |

## The concept

A lambda expression creates an object of a unique, unnamed **closure
type**: a class with the captures as data members and an `operator()`.

```cpp
int fee = 2;
auto add_fee = [fee](int px) { return px + fee; };
// is roughly
struct __closure { int fee; int operator()(int px) const { return px + fee; } };
auto add_fee = __closure{fee};
```

Everything about captures follows from that picture:

| Capture | Member | Notes |
|---|---|---|
| `[x]` | copy of `x`, taken **when the lambda is created** | later changes to `x` are not seen |
| `[&x]` | reference to `x` | dangles if the lambda outlives `x` |
| `[=]`, `[&]` | capture whatever is used, by copy / reference | `[=]` in a member function captures `this` (a pointer!), deprecated in C++20 |
| `[this]` | the `this` pointer | members are read through it: dangles with the object |
| `[*this]` | a copy of the whole object (C++17) | safe to outlive the object |
| `[p = std::move(ptr)]` | init-capture: any expression, any name | how you capture move-only types |
| `[...xs = std::forward<Ts>(args)]` | pack init-capture (C++20) | |

**`operator()` is `const` by default**, so by-copy captures cannot be
modified. `mutable` removes the `const`: the lambda can then keep state
between calls (a counter, a cache). Each *copy* of the lambda has its own
state.

**Generic lambdas** (`auto` parameters) have a templated `operator()`.
C++20 adds explicit template parameter lists: `[]<class T>(const std::vector<T>& v) { ... }`.

A capture-less lambda converts to a plain function pointer (useful for C
callbacks). Since C++20 capture-less lambdas are also default-constructible
and assignable.

**Immediately-invoked lambdas** initialise a `const` variable with logic
that needs statements:

```cpp
const auto tick = [&] {
    if (venue == "XNAS") return 0.01;
    if (px < 1.0) return 0.0001;
    return 0.01;
}();
```

**`std::function`** can store any copyable callable with the right
signature, at the cost of type erasure (possible heap allocation, indirect
call). It requires the callable to be **copyable**: a lambda that captured
a `std::unique_ptr` cannot be stored in it (C++23 adds
`std::move_only_function`). In templates, take callables as `F&&` / `auto`
instead.

## Your task

In `lambdas.hpp` (namespace `idioms`):

1. `make_counter(start)`: returns a lambda; each call returns `start`,
   `start+1`, ... Copies of the lambda count independently.
2. `make_multipliers(n)`: `std::vector<std::function<int(int)>>` where
   element `i` multiplies its argument by `i`. (The classic bug captures
   the loop variable by reference.)
3. `compose(fs...)`: `compose(f, g, h)(x) == f(g(h(x)))`; `compose(f)` is `f`.
4. `memoize(f)`: for `f: long -> long`, returns a callable caching results
   in an `std::unordered_map` it owns. The tests count calls to `f`.
5. `make_owner(std::unique_ptr<int>)`: returns a lambda that owns the
   pointer and returns `*p` when called.
6. `Order::printer() const`: returns a `std::function<std::string()>` that
   prints `"<id>@<price>"` and keeps working after the `Order` is
   destroyed.
7. `tick_size(venue, price)`: written as one `const` variable initialised by
   an immediately-invoked lambda, then returned: `"XNAS"` → `0.01` if price
   `>= 1.0` else `0.0001`; `"XLON"` → `0.5`; anything else → `0.01`.

<details><summary>Hints</summary>

- `make_counter`: `[n = start]() mutable { return n++; }`.
- `compose`: recursion with a pack: `compose(f, rest...)` returns
  `[=](auto&& x) { return f(compose(rest...)(std::forward<decltype(x)>(x))); }`.
- `printer`: `[*this]` or capture the two fields by value.
</details>
