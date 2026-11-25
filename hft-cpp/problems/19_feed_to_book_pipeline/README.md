# 19 · Feed-to-book pipeline

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★★★ | 12 (SPSC), 13 (seqlock), 15 (parser), 17 (book) | composing lock-free components, thread lifecycle and shutdown, CPU pinning, busy-polling, end-to-end testing of concurrent systems |

## Why this matters

A real market data stack is a pipeline of threads, each pinned to its own
core and busy-polling, connected by SPSC queues: one thread decodes packets,
another maintains books, others read snapshots to make trading decisions.
This is where the components you built stop being exercises and become a
system. The hard parts are no longer the data structures but the seams:
startup and shutdown, partial messages, who owns what, and proving that
readers on other threads always see a consistent state.

## Task

Implement `pin_current_thread` and `Pipeline` in `pipeline.hpp`, reusing
your `SpscQueue` (12), `Seqlock` (13), `itch::parse` (15), `OrderBook` and
`BookBuilder` (17). Their headers are already included.

```
feed thread                     book thread                      any thread
on_bytes(chunk) ──parse──► SpscQueue<Event> ──► BookBuilder/OrderBook
                                                     │ after every event
                                                     ▼
                                            Seqlock<TopOfBook> ◄── top()
```

## Requirements

1. `on_bytes` accepts arbitrary chunks of the ITCH stream. Messages can be
   split across calls (down to one byte at a time): keep the unconsumed
   tail and prepend it next time. Decoded events go into an `SpscQueue`;
   if it is full, spin until there is room. No locks.
2. The book thread applies events in order through `BookBuilder` (so only
   the configured stock's orders enter the book) and, **after every event**,
   publishes a `TopOfBook` whose `sequence` is the number of events applied
   so far.
3. `top()` is safe from any thread at any time and never returns a torn
   snapshot: the test checks every snapshot it reads against a model of the
   book at that exact sequence number.
4. `stop()` makes the book thread finish every event already pushed, then
   joins it. It is idempotent. The destructor calls it. `start()` after
   `stop()` resumes with the same book.
5. If `book_cpu >= 0` the book thread pins itself with `pin_current_thread`.
   `pin_current_thread` returns false for invalid CPUs.
6. Clean under ASan, UBSan and TSan.

## Running

```bash
./run_tests.sh          # ASan+UBSan, then TSan
./run_tests.sh bench    # end-to-end throughput
```

<details>
<summary>Hints</summary>

- The queue element can be
  `std::variant<itch::AddOrder, itch::OrderExecuted, ...>`. It is trivially
  copyable because every alternative is. Apply it with `std::visit`.
- The SPSC queue object is large (thousands of slots). Put it on the heap
  with `std::make_unique`.
- Shutdown: set an atomic `stop` flag with release. The book thread exits
  only when it sees the flag *and* the queue is empty. Since the feed
  thread is the one calling `stop()`, everything it pushed is already in
  the queue.
- `pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &set)`.
</details>

## Going further

- Feed the pipeline from your UDP receiver and `SequenceTracker` (16)
  instead of `on_bytes`, with a sender process replaying a file.
- Measure the latency from `on_bytes` to the `top()` update with your
  histogram (09): timestamp the event when pushed, record when published.
- Boot with `isolcpus=` (or use `cset shield`) to isolate cores, pin the
  threads there, and compare the tail latency with and without isolation.
- Publish only when the top of book actually changes and measure the effect.

## Reading

- Carl Cook, "When a Microsecond Is an Eternity" (CppCon 2017): the hot path, warming, pinning
- Martin Thompson et al., "Disruptor: High performance alternative to bounded queues" (LMAX technical paper, 2011)
- Red Hat, "Low Latency Performance Tuning for RHEL" (tuned profiles, isolcpus, IRQ affinity)
- `man 3 pthread_setaffinity_np`, `man 7 cpuset`
