# 27 · RAII for state and locks

| Group | Difficulty | Builds on |
|---|---|---|
| I. RAII | ★★★☆☆ | 25, 26 |

## The concept

RAII is not only for memory and handles. Anything with a "do / undo" pair
fits: temporarily changing a setting, bumping a counter of in-flight
requests, holding a lock, entering a tracing span, pinning a CPU.

### Save and restore

```cpp
{
    SaveRestore guard(log_level);   // remembers the current value
    log_level = Level::debug;       // change it for this scope
    run_diagnostics();              // may throw
}                                   // restored on every path
```

### Locks

`std::lock_guard`, `std::unique_lock` and `std::scoped_lock` are RAII:
the mutex is released by the destructor, so an exception cannot leave it
locked. Things to know cold:

- **Name your guard.** `std::lock_guard<std::mutex>{m};` creates a
  temporary that unlocks at the semicolon: the "critical section" is
  empty. Worse, `std::lock_guard<std::mutex>(m);` *declares a new variable
  named `m`* (most vexing parse) and locks nothing at all.
  With CTAD, write `std::lock_guard lock(m);`.
- **Locking two mutexes** in two threads in opposite orders deadlocks:
  thread 1 holds A waits for B, thread 2 holds B waits for A.
  `std::scoped_lock lock(a.m, b.m);` locks any number of mutexes using a
  deadlock-avoidance algorithm (try-and-back-off), so argument order does
  not matter. The C++11 spelling was `std::lock(a, b)` plus two
  `std::unique_lock`s with `std::adopt_lock`/`std::defer_lock`.
- **Self-transfer** (`a` and `b` the same object): locking the same
  `std::mutex` twice is UB. Check identity first.
- `std::unique_lock` is the movable, unlockable-early guard; it is what
  `std::condition_variable::wait` needs.

### Counting guards

A guard that increments a counter and decrements it on destruction tracks
"how many are in flight" exactly, even with early returns and exceptions.
If it is movable, the moved-from guard must not decrement.

## Your task

In `raii_state.hpp` (namespace `idioms`):

1. `SaveRestore<T>`: saves `var`'s value on construction, restores it on
   destruction, unless `commit()` was called. Not copyable or movable.
2. `InFlight`: constructed from a `std::atomic<int>&`, increments it,
   decrements on destruction; movable (source disarmed), not copyable.
3. `Account { std::mutex m; long balance; }` and
   `transfer(from, to, amount)`: thread-safe and deadlock-free with
   `std::scoped_lock`; returns `false` (and changes nothing) if `from` has
   insufficient funds or `from` and `to` are the same account.
4. `total(accounts)`: sum of balances of a `std::vector<Account*>`, taking
   each account's lock while reading it.

The tests run two threads that transfer in opposite directions 200 000
times. If your locking can deadlock, the test hangs and the runner's
timeout kills it.

<details><summary>Hints</summary>

- `SaveRestore`: store `T& ref_`, `T saved_`, `bool active_`.
- `InFlight` move constructor: copy the pointer to the counter, null the source's.
</details>
