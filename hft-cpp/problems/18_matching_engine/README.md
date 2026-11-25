# 18 · Matching engine (price-time priority)

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★★☆ | 07 (object pool), 08 (hash map), 17 (book structure) | exchange matching semantics, order types (limit, market, IOC), priority rules, intrusive FIFO queues, differential testing against a brute-force model |

## Why this matters

The book in problem 17 *observes* an exchange. Here you build the exchange
side: the engine that decides who trades with whom. "Write a matching
engine" is one of the most common take-home and onsite exercises at trading
firms, because it combines careful specification-following with data
structure design: each order must be findable by id in O(1) for cancels
*and* sit in a FIFO queue at its price level for matching.

## Task

Implement `hft::match::MatchingEngine` in `matching_engine.hpp`.

## Rules

1. **Price-time priority.** An incoming buy matches resting sells priced at
   or below its limit, lowest price first; within a price, oldest first.
   Sells mirror this.
2. **Trade price is the maker's price** (the resting order's price), not the
   taker's limit.
3. **Partial fills.** An order can trade against many makers across many
   levels. A maker partially filled keeps its place in the queue.
4. **Remainder.** GTC: unfilled quantity rests at the order's limit price
   (and the book is never left crossed). IOC: the remainder is discarded.
   `submit_market` is an IOC at any price.
5. **Rejections.** `qty == 0` → `RejectedInvalidQty`. An id that is
   currently resting → `RejectedDuplicateId`. Rejected orders have no effect.
6. **Cancel** removes a resting order; returns false if it is not resting.
7. **Modify.** Same price and quantity reduced (or unchanged): update in
   place, keep priority. Any other change: the order loses priority and is
   re-submitted as a new GTC order with the same id (it may trade).
8. `trades()` lists every fill in the order it happened.

## Running

```bash
./run_tests.sh          # unit tests + 30k random operations against a brute-force engine
./run_tests.sh bench
```

<details>
<summary>Hints</summary>

- Straightforward version: `std::map<price, std::list<Order>>` per side
  (bids with `std::greater<>`), plus `unordered_map<id, {side, price,
  list iterator}>` so cancel can erase from the middle of a queue in O(1).
  `std::list` iterators stay valid when other elements are erased.
- Faster version: orders live in your `ObjectPool` (07) and carry `prev`
  / `next` pointers (an *intrusive* doubly linked list per level), the id
  lookup is your `FlatHashMap` (08), and levels are a sorted vector (17).
  No allocation per order.
- Write the matching loop once as a template over the opposite side's
  container and a "does this price cross?" predicate.
</details>

## Going further

- Self-trade prevention: reject or cancel when taker and maker share an
  account id.
- Add stop orders, or an opening auction that computes the single price
  maximising matched volume.
- Emit events through a callback/handler template parameter instead of a
  vector so the engine never allocates on the hot path.

## Reading

- Larry Harris, *Trading and Exchanges*, chapters 5 and 6
- CME Globex and Nasdaq matching algorithm documentation (FIFO vs pro-rata)
- "How to Build a Fast Limit Order Book" (WK Selph gist)
- Brian Nigito, "How to Build an Exchange" (Jane Street tech talk, 2017)
