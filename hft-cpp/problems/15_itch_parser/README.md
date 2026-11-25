# 15 · Zero-copy ITCH-like market data parser

| Difficulty | Builds on | New skills |
|---|---|---|
| ★★★☆☆ | 04 (concepts, dispatch), 05 (integer prices) | binary protocols, endianness, unaligned loads with `memcpy`, strict aliasing, `std::span`, streaming parsers, static dispatch through a handler concept |

## Why this matters

Exchanges publish market data as compact binary messages. Nasdaq's
TotalView-ITCH is the reference example that every HFT engineer has parsed.
The parser sits at the very front of the tick-to-trade path, so it must not
copy, allocate or call virtual functions. It also has to be *correct* about
bytes: wire integers are big-endian and unaligned, and the "obvious" trick of
`reinterpret_cast`-ing the buffer to a packed struct is undefined behaviour
(alignment and strict aliasing) that UBSan will flag.

## Task

Implement `load_be`, `store_be`, the `Handler` concept, `parse`, and the five
`encode` overloads in `itch_parser.hpp`.

## Wire format

Every message is framed as:

| Offset | Size | Field |
|---|---|---|
| 0 | 2 | `length`: bytes that follow (type byte + body), big-endian |
| 2 | 1 | `type` (ASCII) |
| 3 | length − 1 | body |

All integers are **big-endian**, unsigned, and **unaligned**. Body layouts
(offsets relative to the start of the body, i.e. after the type byte):

**`A` Add Order** (length 34)

| Off | Size | Field |
|---|---|---|
| 0 | 8 | timestamp (ns since midnight) |
| 8 | 8 | order_ref |
| 16 | 1 | side: `'B'` or `'S'` |
| 17 | 4 | shares |
| 21 | 8 | stock (ASCII, space padded) |
| 29 | 4 | price (4 implied decimals) |

**`E` Order Executed** (length 29): timestamp(8) · order_ref(8) · executed_shares(4) · match_number(8)

**`X` Order Cancel** (length 21): timestamp(8) · order_ref(8) · cancelled_shares(4)

**`D` Order Delete** (length 17): timestamp(8) · order_ref(8)

**`U` Order Replace** (length 33): timestamp(8) · orig_order_ref(8) · new_order_ref(8) · shares(4) · price(4)

Any other type byte is an unknown message: skip it using its length.

## Parser rules

1. Process complete messages from the front of the buffer, calling the
   handler for each known type, in order.
2. **Incomplete data is not an error.** If fewer than 2 bytes remain, or the
   length says the message extends past the buffer, stop and return. The
   caller keeps the unconsumed tail and calls again when more bytes arrive.
3. `consumed` is the byte count of the complete messages processed;
   `messages` counts them (unknown types included); `unknown` counts the
   skipped ones.
4. **Errors stop parsing** before the offending message, which is not
   counted as consumed: length 0 → `BadLength`; known type whose length
   differs from the table → `BadLength`; side not `'B'`/`'S'` → `BadSide`.
5. Decode with `memcpy` + byte swap (or byte-by-byte shifts). No
   `reinterpret_cast` of wire bytes to structs or integers.
6. `parse` is a template constrained by your `Handler` concept: the handler
   is called directly (inlinable), with no virtual dispatch and no `std::function`.
7. `encode` writes one framed message and returns its size, or 0 if the
   output span is too small.

## Running

```bash
./run_tests.sh          # ASan+UBSan
./run_tests.sh bench    # messages per second
```

The tests build streams with `common/itch_wire.hpp`, an independent
reference encoder.

<details>
<summary>Hints</summary>

- `std::memcpy(&v, p, sizeof v)` compiles to a single (unaligned) load on
  x86. Byte swap with `__builtin_bswap16/32/64` (or `std::byteswap` in C++23)
  when `std::endian::native == std::endian::little`.
- A `switch` on the type byte is fine. Check the length before you read
  the body.
</details>

## Going further

- Download a real Nasdaq ITCH 5.0 sample file (emi.nasdaq.com/ITCH) and
  extend your parser to its full message set. Count messages per type.
- Make `parse` reject messages from a symbol filter early, before decoding
  the full body.
- Benchmark `memcpy`+`bswap` versus manual shifts versus `std::byteswap`, and
  read the assembly for each.

## Reading

- Nasdaq TotalView-ITCH 5.0 specification (PDF on nasdaqtrader.com)
- Shafik Yaghmour, "What is the Strict Aliasing Rule and Why do we care?" (GitHub gist)
- Timur Doumler, "Type punning in modern C++" (CppCon 2019)
- cppreference: [`std::span`](https://en.cppreference.com/w/cpp/container/span),
  [`std::endian`](https://en.cppreference.com/w/cpp/types/endian)
