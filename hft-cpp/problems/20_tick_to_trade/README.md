# 20 · Capstone: tick-to-trade

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★★★ | 09 (histogram), 15 (parser), 17 (book), and the design habits of everything before | putting a whole trading path together, pre-trade risk controls, rate limiting, deterministic time injection, measuring and attacking tail latency |

## Why this matters

"Tick-to-trade" is the time from a market data packet arriving to the
order it triggers leaving the box. It is the number HFT firms compete on.
This capstone wires your parser, book and histogram into a single-threaded
hot path: decode a message, update the book, run a (deliberately simple)
strategy, run the risk checks every regulated firm must have, and encode an
order onto the wire. Then it measures itself.

The strategy is not the point. The point is the plumbing, the risk layer
that keeps a bug from becoming a headline, and the discipline of measuring
p99 and max, not just the average.

## Task

Implement, in `tick_to_trade.hpp`:

1. **`TokenBucket`**: integer token bucket with injected time (no clock
   calls inside), starting full, refilling at `rate_per_sec` up to `burst`.
   Must not overflow after long idle periods.
2. **`RiskChecker`**: the five checks in the documented order. Position is
   updated only by fills (`on_fill`). Rejected orders must not consume a
   rate token.
3. **`encode_enter_order`**: the 19-byte big-endian order message.
4. **`ImbalanceStrategy`**: top-of-book imbalance, edge triggered (fires
   once when the signal turns on or flips, not on every update while it
   stays on). Buy lifts the offer, sell hits the bid.
5. **`TickToTrade`**: the engine. After every ITCH message: update the book
   (only the configured stock enters), ask the strategy, risk-check any
   order against the mid price and the logical receive time, and either
   append the encoded order to `wire_out()` and `sent()`, or record it in
   `rejected()` with the reason. Client order ids count up from 1 for every
   order the strategy produces. Record one latency sample per sent order:
   time from entering `on_bytes` to the end of encoding.

## Running

```bash
./run_tests.sh          # ASan+UBSan
./run_tests.sh bench    # prints the tick-to-trade latency distribution
./run_tests.sh release  # the tests at -O3 -march=native
```

<details>
<summary>Hints</summary>

- Represent the bucket level in "token-nanoseconds": capacity is
  `burst * 1e9`, each nanosecond adds `rate` units, a token costs `1e9`.
  Everything stays in `uint64_t`. Clamp `elapsed` before multiplying.
- `TickToTrade` owns an `OrderBook`, a `BookBuilder`, the strategy and the
  risk checker. A small handler struct that forwards each ITCH callback to
  the builder and then calls a private `after_update()` keeps `parse`
  statically dispatched.
- Reserve `wire_out()` and the result vectors up front so the hot path does
  not reallocate.
</details>

## Going further

This is open-ended. Pick a target and measure every change with the bench:

- Get p99 down: remove allocations on the hot path (check with the
  allocation counter from `common/alloc_counter.hpp`), swap in your faster
  book from 17's "Going further", avoid `double` in the strategy.
- Split it across threads with your pipeline from 19: market data on one
  core, strategy plus risk plus encode on another.
- Add a simulated exchange using your matching engine (18) so fills come
  from real matching instead of `on_fill` calls.
- Add a kill switch: if more than N orders are rejected in a second, stop
  sending and require a manual reset.
- Read about warming the hot path (sending "fake" orders through the code
  to keep it in cache) and try it.

## Reading

- Carl Cook, "When a Microsecond Is an Eternity" (CppCon 2017). Watch it again now.
- David Gross, "When Nanoseconds Matter: Ultrafast Trading Systems in C++" (CppCon 2024)
- SEC Rule 15c3-5 (the Market Access Rule): why pre-trade risk checks are mandatory
- Knight Capital, SEC administrative proceeding (2013): what happens without them
- Nasdaq OUCH 4.2/5.0 specification (the real order entry protocol this message imitates)
