# 01 · UniquePtr

| Difficulty | Builds on | New skills |
|---|---|---|
| ★☆☆☆☆ | none | RAII, move semantics, rule of five, `noexcept`, `[[no_unique_address]]`, constrained templates |

## Why this matters

Every resource in a trading system (a socket, a mapped file, a pool slot, a
lock) needs exactly one owner and a guaranteed release. RAII is how C++ makes
that automatic and exception-safe, and `std::unique_ptr` is the canonical
example. Interviewers ask you to write one because it exercises move
semantics, special member functions, and template details in thirty lines.
It also has a hard performance requirement: with the default deleter it must
cost nothing over a raw pointer.

## Task

Implement `hft::UniquePtr<T, Deleter>`, `hft::DefaultDelete<T>` and
`hft::make_unique<T>(args...)` in `unique_ptr.hpp`.

## Requirements

1. **Ownership.** The pointer owns at most one object and destroys it with
   `Deleter` when the `UniquePtr` is destroyed, reset, or assigned over.
   The deleter is never called with `nullptr`.
2. **Move-only.** Copy construction and copy assignment do not exist. Move
   construction and move assignment transfer ownership and leave the source
   empty. Self move-assignment must not destroy the object.
3. **`noexcept`.** Moves, `get`, `release`, `reset`, `swap` and `operator bool`
   are `noexcept`. (Why does this matter for `std::vector<UniquePtr<T>>`?)
4. **`reset(p)` order.** Store `p` first, *then* delete the old object, exactly
   as the standard specifies. A destructor that looks at its owner during
   destruction must see the new pointer.
5. **Conversions.** `UniquePtr<Derived>` converts (by move) to `UniquePtr<Base>`
   but not the other way. Use a `requires` clause, not SFINAE tricks, and make
   `DefaultDelete<Derived>` convertible to `DefaultDelete<Base>`.
6. **Custom deleters.** Stateful functors and function pointers both work and
   are moved along with the pointer.
7. **Zero overhead.** `sizeof(UniquePtr<T>) == sizeof(T*)` when the deleter is
   an empty class.
8. **`make_unique<T>(args...)`** perfectly forwards its arguments.

Not required: the array form `UniquePtr<T[]>`, `operator<=>`, hashing.

## Running the tests

```bash
./run_tests.sh                 # -O2 with AddressSanitizer + UBSan
./run_tests.sh asan reset      # only tests whose name contains "reset"
```

All tests fail until you implement the header. That is expected.

<details>
<summary>Hints (open only if stuck)</summary>

- `std::exchange(ptr_, nullptr)` is the idiomatic `release()`.
- Implement move assignment as `reset(other.release())` plus moving the
  deleter. Think about why that is self-assignment safe on its own.
- `[[no_unique_address]] Deleter deleter_;` lets an empty member take no space.
  Before C++20 people used the empty base optimisation (inherit from the deleter).
- The converting constructor needs `std::is_convertible_v<U*, T*>`.
</details>

## Going further

- Add `UniquePtr<T[]>` with `operator[]` and `delete[]`.
- Read how libstdc++ implements `std::unique_ptr` (`bits/unique_ptr.h`) and
  compare with yours. Note `__uniq_ptr_impl` and how it stores the deleter.
- Look at the generated assembly for a function taking `UniquePtr<int>` by
  value versus `int*` on Compiler Explorer. Why is passing it by value not free
  under the Itanium ABI? (Search: "unique_ptr is not zero cost Chandler Carruth".)

## Reading

- Scott Meyers, *Effective Modern C++*, Items 17, 18, 23, 25
- cppreference: [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr),
  [`[[no_unique_address]]`](https://en.cppreference.com/w/cpp/language/attributes/no_unique_address)
- CppCon 2019, Chandler Carruth, "There Are No Zero-cost Abstractions"
