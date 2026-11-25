# 09 · Latency histogram and measurement harness

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★☆☆ | 04 (bit tricks), 07 (pre-allocation) | HDR-style log-linear bucketing, percentiles, `rdtsc` and clock calibration, measuring without distorting what you measure |

## Why this matters

In HFT nobody cares about average latency. They care about p99, p99.9 and
the max, because the one slow reaction is the one that loses money. You
cannot store every sample from a system handling millions of events a
second, so you use a histogram with bounded relative error: Gil Tene's
HdrHistogram is the industry standard. From here on you will use your
histogram to measure your own queues (12–14) and your tick-to-trade
pipeline (20).

## Task

Implement `hft::LatencyHistogram`, `hft::read_tsc`, `hft::TscClock` and
`hft::measure` in `latency_histogram.hpp`.

## Requirements

1. **Log-linear buckets.** Values below `2 * 2^bits` get their own bucket.
   Above that, each power-of-two range `[2^k, 2^(k+1))` is split into
   `2^bits` equal sub-buckets. Bucket count is O(bits × log2(max_value)):
   at most 4096 for 1 s in ns with 7 bits.
2. **`record` is O(1)**, allocates nothing, uses no floating point. Bucket
   index comes from bit operations (`std::countl_zero` / `std::bit_width`).
3. **Exact `min`, `max`, `mean`, `count`.**
4. **`percentile(p)`** uses the nearest-rank definition and reports the
   upper end of the bucket (clamped to `[min, max]`), so the result is
   never below the true value and at most `true × 2^-bits` above it.
5. Values above `max_value` are clamped and counted in `overflow_count()`.
   `max_value` up to `UINT64_MAX` must work.
6. `merge` adds another histogram with the same configuration (throws
   `std::invalid_argument` otherwise). `reset` clears.
7. `read_tsc()` returns the x86 timestamp counter. `TscClock::calibrate`
   measures nanoseconds per tick against `steady_clock`.
8. `measure(hist, n, f)` times `n` calls of `f` and records each.

## Running

```bash
./run_tests.sh
```

<details>
<summary>Hints</summary>

- With `sub = 1 << bits`: for `v < 2*sub` the index is `v`. Otherwise let
  `msb = 63 - countl_zero(v)`, `shift = msb - bits`, and the index is
  `shift * sub + (v >> shift)`. Work out why that is contiguous.
- Going back: for index `i >= 2*sub`, `shift = i / sub - 1`, the bucket
  covers `[(i - shift*sub) << shift, ... + (1 << shift) - 1]`.
- Keep the running sum in something wider than `uint64_t` if you allow
  huge values (`long double` or `unsigned __int128`).
</details>

## Going further

- Coordinated omission: if your load generator waits for each response
  before sending the next, slow responses hide the requests that should
  have been sent meanwhile. Watch Gil Tene's talk below and add
  `record_corrected(value, expected_interval)`.
- Measure the overhead of `steady_clock::now()` versus `rdtsc` versus
  `rdtscp` with your own harness. Then read about why `rdtsc` needs a fence
  (`lfence`) to avoid being reordered.
- Check `/proc/cpuinfo` for `constant_tsc` and `nonstop_tsc`. What breaks
  if they are missing?

## Reading

- Gil Tene, "How NOT to Measure Latency" (Strange Loop 2015). Required viewing.
- HdrHistogram design notes: hdrhistogram.org
- Intel, "How to Benchmark Code Execution Times on Intel IA-32 and IA-64" (white paper on `rdtsc`)
- Brendan Gregg, *Systems Performance*, chapter 2 (methodology)
