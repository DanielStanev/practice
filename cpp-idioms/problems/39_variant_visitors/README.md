# 39 · `std::variant`, the `overloaded` idiom and recursive lambdas

| Group | Difficulty | Builds on |
|---|---|---|
| N. Lambdas | ★★★☆☆ | 33, 38 |

## The concept

A market data feed delivers a closed set of message types. Modelling them
as `std::variant<Add, Cancel, Execute, ...>` instead of a class hierarchy
gives you value semantics (no heap allocation, messages live in a
contiguous buffer), and **exhaustiveness**: a visitor that forgets a case
does not compile.

```cpp
using Msg = std::variant<Add, Cancel, Execute>;
std::visit(visitor, msg);   // calls visitor(std::get<I>(msg)) for the active alternative
```

### The `overloaded` idiom

Writing a visitor struct with three `operator()`s is verbose. Instead,
build one from lambdas by inheriting all of them:

```cpp
template <class... Fs>
struct overloaded : Fs... {
    using Fs::operator()...;          // pull every lambda's operator() into one overload set
};
template <class... Fs>
overloaded(Fs...) -> overloaded<Fs...>;   // deduction guide (not needed in C++20 for aggregates, but harmless)

std::visit(overloaded{
    [](const Add& a)    { ... },
    [](const Cancel& c) { ... },
    [](const auto& other) { ... },     // catch-all: a generic lambda is a worse match than an exact one
}, msg);
```

This combines three features: aggregate initialisation of a class with
base classes, pack expansion in `using`-declarations, and class template
argument deduction.

**Overload resolution pitfalls.** The visitor's overloads are resolved
like any call: a `[](long)` lambda will happily accept an `int`
alternative via conversion; a `[](bool)` lambda accepts pointers. Prefer
exact `const T&` parameters.

`std::visit` can return a value: all branches must return the same type.
`std::visit` with several variants calls the visitor with every
combination (useful for double dispatch).

### Recursive lambdas (C++20)

A lambda cannot refer to itself by name (its type is not complete inside
its own body). The C++20 workaround passes itself as a parameter:

```cpp
auto fib = [](auto self, int n) -> long { return n < 2 ? n : self(self, n - 1) + self(self, n - 2); };
fib(fib, 10);
```

(C++23 "deducing this" makes it `[](this auto self, int n)`.)

## Your task

In `visitors.hpp` (namespace `idioms`):

1. `overloaded<Fs...>` with its deduction guide.
2. `describe(const Msg&)`: `"add 5@100.5"`, `"cancel 7"`, `"exec 3x10"`
   (formats in the header comment) with `std::visit` + `overloaded`.
3. `apply_message(Book&, const Msg&)`: `Add` inserts `{id → qty}`, `Cancel` erases
   the id, `Execute` reduces the qty by the executed amount and erases it
   at zero. Use `std::visit` + `overloaded`. (Why not just call it `apply`?
   Because with a `std::map` argument, argument-dependent lookup also finds
   `std::apply`, which then fails to compile deep inside `<tuple>`. Try it once.)
4. `count_by_type(msgs)`: `std::array<std::size_t, 3>` of how many of each
   alternative, using a **single generic lambda** in `std::visit` (no
   `index()`), mapping each type to its index at compile time.
5. `tree_sum(const Node&)`: a `Node` is either a `long` or a vector of
   `Node`s. Sum all leaves using a **recursive lambda** and `std::visit`.

<details><summary>Hints</summary>

- `count_by_type`: inside the generic lambda, `using T = std::decay_t<decltype(m)>;`
  then `if constexpr (std::is_same_v<T, Add>) ...`, or a small
  `index_of` helper from problem 33.
- `tree_sum`: `auto go = [](auto self, const Node& n) -> long { return std::visit(overloaded{...}, n.v); };`
  The inner lambda for the vector case needs `self` captured.
</details>
