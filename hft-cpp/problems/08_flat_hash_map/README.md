# 08 · Open-addressing flat hash map

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★☆☆ | 02 (storage), 04 (power-of-two math), 07 (no-alloc mindset) | open addressing, linear probing, backward-shift deletion, hash mixing, load factor, cache-friendly layout |

## Why this matters

Every order book and matching engine needs "order id → order" lookups on
every cancel, modify and execution. `std::unordered_map` is a linked
structure: one allocation per node and a cache miss per hop. Trading firms
use flat, open-addressing tables (Abseil's `flat_hash_map`, Folly's `F14`,
`ankerl::unordered_dense`) where a lookup is usually one cache line. You will
use your map in problems 17 and 18.

## Task

Implement `hft::FlatHashMap<K, V, Hash, KeyEqual>` in `flat_hash_map.hpp`.

## Requirements

1. **Open addressing with linear probing** in one contiguous slot array.
   Capacity is always a power of two so the bucket index is `hash & mask`.
2. **Load factor** stays at or below 0.875 (0.75 is a good choice). The table
   doubles and rehashes when an insert would exceed it.
3. **Deletion without tombstones** using *backward-shift deletion*: after
   removing an element, move later elements of the probe run back so no
   lookup ever has to skip a "deleted" marker. A table with a fixed working
   set that is churned forever must never grow.
4. **Hash mixing.** `std::hash<uint64_t>` is the identity in libstdc++.
   Apply a bit mixer (e.g. the MurmurHash3 finaliser or a Fibonacci multiply)
   before masking so that strided keys (`i << 12`) spread out.
5. API: `insert` (returns false if present, no change), `insert_or_assign`,
   `operator[]`, `find` (returns `V*`/`nullptr`), `contains`, `erase`,
   `clear` (keeps capacity), `reserve(n)` (no rehash until `n` elements),
   `for_each(f)`, `size`, `capacity`, `load_factor`.
6. Values are constructed and destroyed exactly once each (tests count them).
7. Correct with terrible hash functions (all keys colliding, or only 7
   distinct hashes), including probe runs that wrap around the end.

## Running

```bash
./run_tests.sh          # correctness
./run_tests.sh bench    # vs std::unordered_map
```

<details>
<summary>Hints</summary>

- Simplest slot type: `std::optional<std::pair<K, V>>` in a `std::vector`.
  Faster: a separate byte array of "occupied" flags (or hash fragments)
  next to raw storage, so probing touches less memory.
- Backward shift: after emptying slot `hole`, walk `j = hole+1, hole+2, ...`
  until you hit an empty slot. An element at `j` with home bucket `h` may
  move into `hole` iff `hole` lies cyclically in `[h, j)`, i.e.
  `((j - h) & mask) >= ((j - hole) & mask)`. When it moves, `j` becomes the new hole.
- Mixer: `h ^= h >> 33; h *= 0xff51afd7ed558ccd; h ^= h >> 33;`.
</details>

## Going further

- Robin Hood hashing: on insert, steal the slot from elements closer to
  their home. It bounds probe-length variance.
- Store a 7-bit hash fragment per slot and compare 16 at a time with SSE2
  (the Swiss table design).
- Add a heterogeneous `find(std::string_view)` for `std::string` keys.

## Reading

- Matt Kulukundis, "Designing a Fast, Efficient, Cache-friendly Hash Table, Step by Step" (CppCon 2017)
- Malte Skarupke, "I Wrote the Fastest Hashtable" (probablydance.com)
- Emmanuel Goossaert, "Robin Hood hashing: backward shift deletion"
- Martin Ankerl's hash map benchmarks (martin.ankerl.com)
