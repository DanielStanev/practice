# 12 · Lock-free SPSC ring buffer

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★★☆ | 02 (raw storage), 04 (power of two), 11 (acquire/release, baseline numbers) | lock-free programming, the release/acquire publication pattern, false sharing and cache-line padding, cached indices, wait-free progress |

## Why this matters

The single-producer single-consumer queue is the most important data
structure in a low-latency system. It connects the feed handler thread to
the strategy thread, the strategy to the order gateway, and the hot path to
the logger. A good one hands off an item in tens of nanoseconds with no
locks, no syscalls and no allocation. You will reuse it in problems 19 and 20.
"Implement an SPSC queue and explain the memory orderings" is probably the
single most common HFT C++ interview question.

## Task

Implement `hft::SpscQueue<T, Capacity>` in `spsc_queue.hpp`.

## Requirements

1. **Ring buffer** of `Capacity` (power of two) elements in raw storage
   inside the object. No heap allocation. No default construction of
   slots: `T` need not be default constructible.
2. **Exactly `Capacity` elements fit.** (Hint: monotonically increasing
   64-bit head and tail counters masked on access, instead of wrapping
   indices that "waste" one slot.)
3. **Lock-free and wait-free**: no mutexes, no CAS loops. Producer only
   writes `tail`, consumer only writes `head`.
4. **Correct memory ordering**: publishing an element is a `release` store
   of `tail`, observing it is an `acquire` load. Same in the other direction
   for freeing slots. Everything else is `relaxed`. Explain each choice in a
   comment. TSan must be clean.
5. **No false sharing**: `head` and `tail` live on different cache lines
   (`alignas(64)`), and the queue object itself is cache-line aligned.
6. **Cached indices**: the producer keeps a local copy of `head` and only
   re-reads the shared one when the queue looks full; the consumer does the
   same with `tail`. This removes most cross-core traffic.
7. `front()` / `pop()` let the consumer work on an element in place.
   `try_pop(out)` moves it out. The destructor destroys remaining elements.

## Running

```bash
./run_tests.sh          # ASan+UBSan, then TSan
./run_tests.sh bench    # throughput and ping-pong round trip
```

Compare the throughput with `BlockingQueue` from problem 11. You should see
an order of magnitude or more.

<details>
<summary>Hints</summary>

- Producer: `t = tail.load(relaxed)`; if `t - head_cache == Capacity`,
  refresh `head_cache = head.load(acquire)` and fail if still full; construct
  into slot `t & mask`; `tail.store(t + 1, release)`.
- Consumer mirrors it with `head`, `tail_cache` and `tail.load(acquire)`.
- Why must the producer's `head` load be `acquire`? Because the consumer
  may have just destroyed/moved from that slot, and the producer is about
  to overwrite it.
- Put `head` and the consumer's `tail_cache` together on one cache line,
  `tail` and `head_cache` on another.
</details>

## Going further

- Pin producer and consumer to two physical cores (`taskset` or
  `pthread_setaffinity_np`) and rerun. Then pin them to two hyperthreads of
  the same core. Explain the difference.
- Batch: let the consumer process everything available before publishing
  `head` once. Measure.
- Read Erik Rigtorp's `SPSCQueue` and Folly's `ProducerConsumerQueue`, and
  compare with yours.

## Reading

- Erik Rigtorp, "Optimizing a ring buffer for throughput" (rigtorp.se)
- Herb Sutter, "Lock-Free Programming (or, Juggling Razor Blades)" (CppCon 2014)
- Martin Thompson, LMAX Disruptor paper and "Mechanical Sympathy" blog
- Jeff Preshing, "Acquire and Release Semantics" and "The Synchronizes-With Relation" (preshing.com)
