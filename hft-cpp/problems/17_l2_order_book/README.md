# 17 · Price-level (L2) order book

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★★☆ | 05 (prices), 08 (id lookup), 10 (layout), 15 (ITCH messages) | order book semantics (L3 → L2), choosing containers for sorted price levels, keeping two indexes consistent, testing against a reference model |

## Why this matters

The order book is the core data structure of every trading system. An
exchange feed like ITCH is *order-level* (L3): it tells you about individual
orders being added, executed, cancelled and replaced. Strategies mostly want
the *price-level* (L2) view: how much is bid and offered at each price, and
especially the best bid and ask. Building and maintaining that view as fast
as messages arrive is a classic HFT task and a classic interview exercise.
Problem 19 runs this book on its own thread; problem 20 trades off it.

## Task

Implement `OrderBook` and `BookBuilder` in `order_book.hpp`.

## Requirements

1. Keep every resting order by `ref` (side, price, remaining qty) and the
   aggregated levels for each side (total qty and order count per price).
   The two must always agree.
2. `add`, `execute`, `cancel`, `remove`, `replace` follow ITCH semantics:
   execute/cancel reduce an order and remove it at zero; replace removes the
   old order and adds a new one with the same side at a new price/qty (it
   loses time priority, which does not matter for an L2 book).
3. Unknown refs return `false` and change nothing. In a real feed you see
   messages for orders you filtered out (other stocks), so this is normal.
4. Empty levels disappear: `depth()` counts only levels with orders.
5. `best_bid` / `best_ask` / `levels(side, out)` return levels best first.
6. `BookBuilder` is an `itch::Handler` (problem 15) that applies decoded
   messages to the book, filtering Add Orders by stock.
7. Every operation is O(log levels) or better. Run the benchmark and try
   to beat the reference numbers in the "Going further" section.

## Running

```bash
./run_tests.sh          # includes a 100k-step randomized comparison with a naive model
./run_tests.sh bench    # realistic message mix, ns per message
```

<details>
<summary>Hints</summary>

- A simple, correct start: `std::unordered_map<ref, Order>` plus
  `std::map<price, Level, std::greater<>>` for bids and
  `std::map<price, Level>` for asks. Get the tests passing with that.
- Then optimise: most activity is within a few ticks of the best price. A
  `std::vector` of levels sorted with the best price at the *back* makes
  inserts and removals near the top cheap (few elements to shift) and is
  cache friendly. Your `FlatHashMap` from problem 08 can replace
  `unordered_map`.
</details>

## Going further

- Implement the vector-of-levels version and compare with the map version
  using the benchmark. The reference `std::map` + `std::unordered_map`
  solution measures roughly 70 ns per message on a cloud VM; a good
  vector-of-levels plus flat-hash-map version should be several times faster.
- Array indexed by price tick (`price - base_price`) for instruments with a
  narrow price range: O(1) everything. What happens when the price moves
  outside the array?
- Load a real ITCH file through your parser (15) and `BookBuilder` and
  print AAPL's top of book every second of the trading day.

## Reading

- Larry Harris, *Trading and Exchanges*, chapters 4–7 (order books, order types)
- WK Selph, "How to Build a Fast Limit Order Book" (gist, classic)
- David Gross, "When Nanoseconds Matter: Ultrafast Trading Systems in C++" (CppCon 2024)
- Carl Cook, "When a Microsecond Is an Eternity: High Performance Trading Systems in C++" (CppCon 2017)
