# 11 · Spinlock and blocking queue (the baseline)

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★☆☆ | 03 (atomics), 09 (measuring) | acquire/release ordering, test-and-test-and-set, `PAUSE`, Lockable requirements, `std::condition_variable`, spurious wakeups, shutdown protocols, ThreadSanitizer |

## Why this matters

Before writing lock-free queues you need the thing they replace and a
number to beat. A mutex plus condition variable queue is simple and
correct, but every hand-off can involve a futex syscall and a context
switch (microseconds). A spinlock trades CPU for latency: it never sleeps,
which is the right call on a dedicated, isolated core and the wrong one
anywhere else. Knowing when each is appropriate, and *why* a spinlock needs
acquire/release semantics, is basic HFT interview material.

## Task

Implement `hft::cpu_relax`, `hft::Spinlock` and `hft::BlockingQueue<T>` in
`spinlock_and_blocking_queue.hpp`.

## Requirements

### Spinlock
1. `lock`, `try_lock`, `unlock`, usable with `std::lock_guard` and friends.
2. **Test-and-test-and-set**: while the lock is held, spin on a plain
   relaxed *load* (which keeps the cache line shared) and only attempt the
   `exchange` when it looks free. Call `cpu_relax()` (x86 `_mm_pause()`) in
   the spin loop.
3. Correct memory ordering: `acquire` on lock, `release` on unlock. TSan
   verifies that data protected by the lock is race-free.
4. `sizeof(Spinlock) <= 8`.

### BlockingQueue
5. Bounded FIFO for many producers and many consumers.
6. `push` blocks while full; `pop` blocks while empty; `try_push`/`try_pop`
   never block; `pop_for(timeout)` blocks at most `timeout`.
7. `try_push(std::move(x))` leaves `x` intact when it fails.
8. **Shutdown**: `close()` wakes every blocked thread. After close, pushes
   fail, pops return remaining items and then `nullopt`.
9. Handle spurious wakeups (use the predicate overloads of `wait`).

## Running

```bash
./run_tests.sh          # ASan+UBSan, then TSan
./run_tests.sh bench    # mutex vs spinlock, and queue hand-off cost
```

Write down the `BlockingQueue 1 producer -> 1 consumer` number. You will
compare it with problem 12.

<details>
<summary>Hints</summary>

- Notify *after* unlocking the mutex so the woken thread does not
  immediately block on it.
- Two condition variables (`not_empty`, `not_full`) avoid waking the wrong
  kind of waiter.
- `std::deque<T>` is fine for storage here; the point is the
  synchronisation. You will build a ring buffer in problem 12.
</details>

## Going further

- Add exponential backoff to the spinlock and benchmark 4 and 8 contending
  threads. Then read why spinlocks in user space are dangerous without
  dedicated cores (Linus Torvalds' 2020 posts on the topic).
- Implement a ticket lock (fair, FIFO). Compare fairness and throughput.
- Run the benchmark under `perf stat -e context-switches`.

## Reading

- Anthony Williams, *C++ Concurrency in Action* (2nd ed.), chapters 3, 4 and 5
- Herb Sutter, "atomic<> Weapons" (C++ and Beyond 2012), parts 1 and 2
- Fedor Pikus, "C++ atomics, from basic to advanced" (CppCon 2017)
- cppreference: [`std::condition_variable`](https://en.cppreference.com/w/cpp/thread/condition_variable)
