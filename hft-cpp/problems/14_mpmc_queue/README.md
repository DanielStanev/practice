# 14 · Bounded lock-free MPMC queue

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★★☆ | 12 (ring buffer, release/acquire), 07 (aligned storage) | compare-and-swap loops, per-slot sequence numbers, ABA avoidance, lock-free vs. wait-free, contention costs |

## Why this matters

When several threads produce work (multiple feed lines, several strategies
sending orders to one gateway) you need a multi-producer queue. Dmitry
Vyukov's bounded MPMC queue is the classic design: each slot carries a
sequence number that says whose turn it is, so producers and consumers
coordinate with a single CAS on a shared position and never touch a lock.
Understanding why the per-slot sequence number makes the ABA problem go
away is a common senior-level interview topic.

## Task

Implement `hft::MpmcQueue<T>` in `mpmc_queue.hpp`.

## Requirements

1. Capacity is a power of two ≥ 2 given at construction (else
   `std::invalid_argument`). All storage is allocated in the constructor.
   Exactly `capacity` elements fit.
2. `try_push` / `try_emplace` / `try_pop` are safe from any number of
   threads, lock-free, and never block. They return false when full / empty.
3. Every pushed element is popped exactly once. Items from one producer
   are popped in the order it pushed them.
4. Elements are constructed in place and destroyed exactly once; the
   destructor destroys elements still queued.
5. The enqueue and dequeue positions live on separate cache lines.
6. TSan clean.

## Running

```bash
./run_tests.sh          # ASan+UBSan, then TSan
./run_tests.sh bench    # 1x1 and 2x2 throughput
```

<details>
<summary>Hints</summary>

The algorithm, per cell `c` with sequence `seq`, initialised to its index:

- **Push** at position `pos = enqueue_pos.load(relaxed)`:
  `diff = c.seq.load(acquire) - pos`.
  - `diff == 0`: the cell is free for this lap. CAS `enqueue_pos` from
    `pos` to `pos + 1`; if you win, construct the element and
    `c.seq.store(pos + 1, release)`.
  - `diff < 0`: the queue is full.
  - `diff > 0`: another producer got here first; reload `pos` and retry.
- **Pop** mirrors it with `dequeue_pos`, expects `seq == pos + 1`, and after
  moving the element out stores `seq = pos + capacity` (free for the next lap).

Compute `diff` as a signed integer (`intptr_t`) so wrap-around works.
</details>

## Going further

- Pad each cell to a cache line and measure the 2x2 throughput change.
- Compare with `boost::lockfree::queue` and `moodycamel::ConcurrentQueue`.
- Explain why this queue is lock-free but not wait-free, and what happens
  if a producer is descheduled between winning the CAS and publishing `seq`.

## Reading

- Dmitry Vyukov, "Bounded MPMC queue" (1024cores.net)
- Erik Rigtorp's `MPMCQueue` (github.com/rigtorp/MPMCQueue)
- Maurice Herlihy and Nir Shavit, *The Art of Multiprocessor Programming*, chapters 3 and 10
- Fedor Pikus, "The Speed of Concurrency (Is Lock-free Faster?)" (CppCon 2016)
