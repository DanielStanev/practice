# 10 · AoS vs SoA order store

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★☆☆ | 05 (integer prices), 08 (id lookup), 09 (measuring) | data-oriented design, cache lines and memory bandwidth, swap-and-pop removal, branch-free loops, auto-vectorisation, reading benchmark results |

## Why this matters

The CPU fetches memory in 64-byte cache lines. If you scan an array of
64-byte order records to sum one 4-byte field, you pull 16× more memory
through the cache than you use. Storing each field in its own array
(Struct of Arrays) fixes that and lets the compiler use SIMD. This
"data-oriented design" mindset is how HFT engineers think about every hot
data structure, and "how would you lay this out in memory?" is a standard
interview follow-up.

## Task

Implement two classes with the same interface in `order_store.hpp`:

- `AosOrderStore`: one `std::vector<Order>` of ~64-byte records.
- `SoaOrderStore`: one vector per field (`ids`, `sides`, `prices`, `qtys`, `timestamps`).

Then run the benchmark and explain the numbers to yourself.

## Requirements

1. `add` rejects duplicate ids and zero quantities.
2. `remove` is O(1): move the last element into the hole (*swap and pop*)
   and fix up the id → index map. Order of elements is not preserved.
3. `modify_qty(id, 0)` removes the order.
4. `get` returns an `OrderView` or `nullopt`.
5. Scans: `total_qty(side)`, `qty_at_or_better(side, limit)` (buy: price ≥
   limit, sell: price ≤ limit), `best_price(side)`, `count_older_than(ts)`.
6. Both layouts give identical answers (the tests run every case on both
   and compare against a reference model).
7. Run `./run_tests.sh bench` and get SoA scans clearly faster than AoS at
   one million orders.

## Running

```bash
./run_tests.sh          # correctness for both layouts
./run_tests.sh bench    # AoS vs SoA scans at 10k and 1M orders
```

<details>
<summary>Hints</summary>

- If your first benchmark shows little difference, look at the scan loop.
  With random sides, `if (side == s) sum += qty;` mispredicts half the
  time and that cost dominates. Make the body branch-free, e.g.
  `sum += qty * uint64_t(side == s)`, and compare again for both layouts.
- Check vectorisation with `-fopt-info-vec` (GCC) or `-Rpass=loop-vectorize`
  (Clang), or read the loop in Compiler Explorer.
- `count_older_than` only touches the `timestamps` array in SoA. How many
  bytes per order does each layout stream from memory for it?
</details>

## Going further

- Use `perf stat -e cache-misses,branch-misses ./build/order_store_bench`
  to confirm your explanation with hardware counters.
- Try a hybrid: keep `side` and `price` together, everything else apart.
- Replace `std::unordered_map` with your `FlatHashMap` from problem 08
  and measure `add`/`remove` throughput.

## Reading

- Mike Acton, "Data-Oriented Design and C++" (CppCon 2014)
- Ulrich Drepper, "What Every Programmer Should Know About Memory"
- Scott Meyers, "CPU Caches and Why You Care" (code::dive 2014)
- Richard Fabian, *Data-Oriented Design* (free online book)
