# 13 · Seqlock

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★★☆ | 11, 12 (acquire/release) | sequence locks, optimistic reads, standalone fences (`std::atomic_thread_fence`), why "benign" data races are UB in C++, torn reads |

## Why this matters

A market data thread updates the top of book millions of times a second
and several strategy threads want the latest value. A mutex would let a
slow reader delay the writer, and the writer is the one thing that must
never wait. A seqlock flips the cost onto readers: the writer bumps a
sequence number, writes, bumps it again; readers copy optimistically and
retry if the number changed. The Linux kernel uses it for the clock, and
HFT shops use it for shared snapshots. You will use it in problem 19.

It is also famous for being easy to get *subtly* wrong in C++. Copying the
payload with plain `memcpy` while the writer modifies it is a data race,
which is undefined behaviour, even though the reader throws the copy away.

## Task

Implement `hft::Seqlock<T>` in `seqlock.hpp` for trivially copyable `T`.

## Requirements

1. `store` (single writer) never blocks and never waits for readers.
2. `load` returns a snapshot equal to some value that was stored: never a
   mix of two writes. `try_load` makes one attempt and reports failure.
3. Readers never observe values going backwards in time.
4. `version()` is the number of completed stores.
5. **No data races by the C++ memory model**: ThreadSanitizer must be
   clean. That rules out reading the payload with a plain `memcpy` while
   it is being written. Store the payload in an array of
   `std::atomic<std::uint64_t>` words and copy them with `relaxed` loads
   and stores, ordered by fences around the sequence counter.
6. Works for sizes that are not multiples of 8.

> Note: ThreadSanitizer does not model `std::atomic_thread_fence`, so it
> cannot prove your fences are right. It *will* catch plain (non-atomic)
> payload reads racing with writes. Reason about the fences yourself.

## Running

```bash
./run_tests.sh          # ASan+UBSan, then TSan
```

<details>
<summary>Hints</summary>

- Writer: `s = seq.load(relaxed); seq.store(s+1, relaxed);
  atomic_thread_fence(release); write words (relaxed); seq.store(s+2, release);`
- Reader: `s1 = seq.load(acquire)`; if odd, fail; read words (relaxed);
  `atomic_thread_fence(acquire); s2 = seq.load(relaxed)`; succeed iff `s1 == s2`.
- Why does the writer need the release *fence* after the first increment
  and not just a release store? Think about which writes must not be
  reordered before which.
</details>

## Going further

- Read Hans Boehm's paper below. It is the definitive treatment of why
  this is hard and which fences you need.
- Benchmark readers per second with 1, 2 and 4 reader threads against a
  `std::shared_mutex` protected value.
- Put the sequence counter and the payload on the same cache line versus
  separate lines and measure the reader retry rate.

## Reading

- Hans-J. Boehm, "Can Seqlocks Get Along With Programming Language Memory Models?" (HP Labs, 2012)
- Linux kernel `include/linux/seqlock.h`
- Erik Rigtorp, "Seqlock" (rigtorp.se) and his `Seqlock.h`
- P1478 "Byte-wise atomic memcpy" (the standard's proposed fix)
