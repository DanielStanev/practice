# 35 · CRTP static dispatch: a feed handler framework with no virtual calls

| Group | Difficulty | Builds on |
|---|---|---|
| L. CRTP | ★★★★☆ | 32, 34 |

## The concept

A market data framework parses messages and calls user code for each
one. The textbook design is an abstract base with virtual hooks:

```cpp
struct Handler { virtual void on_add(const Add&) {} virtual void on_trade(const Trade&) {} ... };
```

Every message then costs an indirect call that the compiler cannot inline,
on the hottest path in the system. Most trading code uses **CRTP** instead:
the framework is a template on the user's class, and hooks are resolved at
compile time.

```cpp
template <class Derived>
class FeedHandler {
public:
    void process(const Message& m) {
        switch (m.type) {
            case 'A': derived().on_add(m.add); break;     // direct call: inlinable
            case 'T': derived().on_trade(m.trade); break;
        }
    }
    // Defaults: used when Derived does not declare its own.
    void on_add(const Add&) { ++ignored_; }
    void on_trade(const Trade&) { ++ignored_; }
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
};

struct MyStrategy : FeedHandler<MyStrategy> {
    void on_trade(const Trade& t);   // "overrides" by hiding the base version
};
```

`derived().on_add(...)` does normal name lookup in `Derived`: if
`MyStrategy` declares `on_add`, that is found; otherwise lookup continues
into the base and finds the default. This is **name hiding**, not
overriding, and it is resolved entirely at compile time.

### Pitfalls and how to guard against them

- **Typos silently fall back to the default.** `void on_trades(...)`
  compiles fine and your handler is never called. A concept check in the
  framework can require that certain hooks are provided by `Derived`
  itself. A way to tell "declared in Derived" from "inherited": compare
  member pointer types. `&Derived::on_trade` has type
  `void (Base::*)(const Trade&)` if it was inherited, and
  `void (Derived::*)(const Trade&)` if Derived declared it.
- **Signature mismatches** (`on_trade(Trade)` by value, or a `const`
  member) also compile and are called. Usually fine; decide whether you care.
- You **cannot** store different handlers in one container (no common
  base type). When you need that, put a type-erased or virtual layer
  *around* the hot loop, not inside it.
- C++23 *deducing this* (`void process(this auto& self, ...)`) replaces most
  CRTP with less boilerplate. On C++20 code bases CRTP is still the norm.

## Your task

In `feed_handler.hpp` (namespace `idioms`):

1. `FeedHandler<Derived>`:
   - `void process(const Message& m)` dispatches on `m.type`:
     `'A'` → `on_add(m.add)`, `'X'` → `on_cancel(m.cancel)`,
     `'T'` → `on_trade(m.trade)`; any other type → `on_unknown(m)`.
   - Default hooks for all four that increment `ignored()`.
   - `void process_all(const std::vector<Message>&)`.
   - `ignored()` count, accessible to tests.
   - No virtual functions anywhere (the test checks
     `!std::is_polymorphic_v`).
2. `overrides_on_trade_v<D>`: true when `D` itself declares
   `on_trade(const Trade&)`, false when it is inherited from
   `FeedHandler<D>`.
3. `StrictFeedHandler<Derived>`: same as `FeedHandler`, but `process`
   must not compile unless `Derived` declares its own `on_trade`
   (use `overrides_on_trade_v` in a `static_assert` or constraint inside
   a member function, since `Derived` is incomplete when the base is
   instantiated).
4. `VwapTracker`: derives from `FeedHandler<VwapTracker>`, handles only
   trades, and exposes `double vwap() const` (0 if no volume).

<details><summary>Hints</summary>

- `overrides_on_trade_v`:
  `!std::is_same_v<decltype(&D::on_trade), void (FeedHandler<D>::*)(const Trade&)>`.
- Why can't the `static_assert` be at class scope in the base? When
  `struct X : StrictFeedHandler<X>` is being parsed, `X` is incomplete,
  so `&X::on_trade` cannot be named yet. Inside a member function body
  it is fine: bodies are instantiated later, when `X` is complete.
</details>
