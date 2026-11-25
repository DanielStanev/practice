# 41 · Bytes, type punning, alignment and strict aliasing

| Group | Difficulty | Builds on |
|---|---|---|
| O. Lifetime and undefined behaviour | ★★★☆☆ | 36, 40 |

## The concept

Market data arrives as bytes. The tempting way to read a binary message is
to point a struct at the buffer:

```cpp
struct Header { std::uint16_t length; std::uint8_t type; std::uint64_t seq; };
auto* h = reinterpret_cast<const Header*>(buf + offset);   // UB, twice
```

Two separate rules make that undefined:

1. **Strict aliasing.** You may only access an object through an lvalue of
   its own type (or a few exceptions: `char`, `unsigned char`,
   `std::byte`, the signed/unsigned twin). There is no `Header` object in
   that buffer, only bytes. The optimiser assumes a `Header*` and a
   `std::byte*` do not alias and may reorder or drop your reads.
2. **Alignment.** `std::uint64_t` usually needs 8-byte alignment. Wire
   protocols pack fields with no padding, so fields land at odd offsets.
   x86 tolerates misaligned loads (slower, and not for every instruction:
   aligned SIMD loads fault), ARM may not. UBSan reports it
   (`-fsanitize=alignment`, part of `undefined`).

### The correct tools

- `std::memcpy(&value, ptr, sizeof value)`: copying bytes into a real
  object is always allowed. Compilers recognise fixed-size `memcpy` and
  emit a single (possibly unaligned) load: **zero cost**.
- `std::bit_cast<To>(from)` (C++20, `<bit>`): reinterpret the bits of one
  trivially copyable object as another of the same size, as a value.
  `constexpr`-friendly. The right way to get a `double`'s bits.
- `std::endian::native` (C++20) tells you the platform byte order;
  network protocols are big-endian, most CPUs little-endian. (`std::byteswap`
  is C++23; write your own here.)
- Reading through `const std::byte*` or `const unsigned char*` is always
  fine: byte access is the aliasing exception.
- C++23 adds `std::start_lifetime_as<T>` for "this memory really does
  contain a T" (zero-copy reading). Until then, memcpy.

Also note: struct layout includes **padding**, so `sizeof(Header)` above
is 16, not 11. Do not memcpy a whole wire message into a struct unless you
control its layout (`#pragma pack` is non-standard).

## Your task

In `bytes.hpp` (namespace `idioms`). No `reinterpret_cast` to anything
other than `std::byte`/`unsigned char`/`char` pointers; the tests run under
UBSan, which reports misaligned accesses.

1. `load<T>(const std::byte* p)`: read a trivially copyable `T` from any
   address (native byte order).
2. `store<T>(std::byte* p, T v)`.
3. `byteswap(x)` for `std::uint16_t`, `std::uint32_t`, `std::uint64_t`
   (one template constrained on unsigned integers), `constexpr`.
4. `load_be<T>(p)` / `load_le<T>(p)`: read an unsigned integer in big/little
   endian, correct on any host (use `std::endian::native`).
5. `double_bits(double)` → `std::uint64_t` and `bits_double(std::uint64_t)`
   → `double`, with `std::bit_cast`. Both `constexpr`.
6. `parse_header(buf, offset)`: wire format (big-endian, packed):
   `u16 length, u8 type, u64 seq, i32 price_ticks` at `buf + offset`
   (offset may be odd). Return a `Header` struct.

<details><summary>Hints</summary>

- `load`: `T v; std::memcpy(&v, p, sizeof(T)); return v;` (`T` must be
  default constructible here; use `std::array<std::byte, sizeof(T)>` and
  `std::bit_cast` if you want to avoid that).
- `byteswap` for any width: loop over bytes with shifts, or use
  `__builtin_bswap*`. A loop compiles to a single `bswap` instruction at -O2.
- Signed `i32` from big-endian: `load_be<std::uint32_t>` then
  `static_cast<std::int32_t>` (well-defined modular conversion since C++20).
</details>
