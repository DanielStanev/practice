# 25 · RAII scope guards: exit, fail, success

| Group | Difficulty | Builds on |
|---|---|---|
| I. RAII | ★★☆☆☆ | 20 |

## The concept

**RAII** (Resource Acquisition Is Initialisation) ties a cleanup action to
an object's destructor. Destructors run on every path out of a scope:
normal exit, `return`, `break`, and exceptions (stack unwinding). So
cleanup written in a destructor cannot be forgotten on the error path,
which is where hand-written cleanup usually goes wrong.

For one-off cleanups you do not want a whole class. A **scope guard**
wraps a lambda and runs it in its destructor:

```cpp
void publish(Book& book, Order o) {
    book.add(o);
    auto undo = ScopeFail([&] { book.remove(o.id); });   // only if we leave by exception
    risk.check(o);           // may throw -> the order is removed again
    gateway.send(o);         // may throw -> same
}                            // success: the guard does nothing
```

Three flavours (they were proposed for the standard as
`std::experimental::scope_exit/scope_fail/scope_success`):

| Guard | Runs on scope exit when |
|---|---|
| `ScopeExit` | always (unless dismissed) |
| `ScopeFail` | leaving because of an exception |
| `ScopeSuccess` | leaving normally |

How does a destructor know whether an exception is in flight?
`std::uncaught_exceptions()` returns how many exceptions are currently being
propagated. Record it in the constructor; if it is **larger** in the
destructor, this scope is being unwound by a new exception. (The old
`std::uncaught_exception()`, singular, gets this wrong when a guard is used
inside a destructor that runs during another unwinding, which is why it was
removed in C++20.)

Rules for writing guards:

- The guard is **not copyable** (two copies would run the action twice) but
  should be **movable** so factory functions can return it; a moved-from
  guard must not run.
- `release()` (or `dismiss()`) disarms it: "commit" semantics.
- `ScopeExit` and `ScopeFail` destructors are called during unwinding:
  if the action throws then, the program terminates. Keep them
  `noexcept`-safe. `ScopeSuccess` may throw from its destructor (it only runs
  when no exception is active), so its destructor is
  `noexcept(noexcept(action()))`.
- Constructor template argument deduction (CTAD) lets callers write
  `ScopeExit guard([&]{ ... });` with no `make_` function.

## Your task

Implement in `scope_guard.hpp` (namespace `idioms`):

1. `ScopeExit<F>`, `ScopeFail<F>`, `ScopeSuccess<F>`: constructor from the
   callable (forwarding it in), non-copyable, movable (the source is
   released), `release()`. The deduction guides are given in the stub;
   read them, they are a one-line idiom worth knowing.
2. `ScopeSuccess`'s destructor may throw if the action throws.
3. `transfer(a, b, amount, fail_after_debit)`: debit `a`, credit `b`. If
   `fail_after_debit` is true, throw `std::runtime_error` between the two
   steps. Use a `ScopeFail` so an exception leaves both balances unchanged.

<details><summary>Hints</summary>

- Store `F fn_`, `bool active_ = true` and, for fail/success,
  `int exceptions_ = std::uncaught_exceptions();`.
- Move constructor: `fn_(std::move(other.fn_)), active_(std::exchange(other.active_, false))`.
</details>
