# 21 · `noexcept`, `move_if_noexcept` and implicit moves on return

| Group | Difficulty | Builds on |
|---|---|---|
| G. Value categories and moves | ★★★☆☆ | 19, 20 |

## The concept

### Why `std::vector` sometimes copies when it could move

When a vector grows it relocates its elements to a new buffer. It promises
the **strong exception guarantee** for `push_back`: if anything throws, the
vector is unchanged. If it *moves* elements and the 5th move throws, the
first 4 sources are already plundered and cannot be restored. So the vector
only moves if the move constructor is `noexcept` (or if the type cannot be
copied at all); otherwise it **copies**. That decision is
`std::move_if_noexcept(x)`, which returns `T&&` or `const T&` accordingly.

Consequence: a type whose move constructor is not marked `noexcept` makes
every vector growth a deep copy. This is one of the most common silent
performance bugs in C++ code bases.

When are moves `noexcept`?

- `= default` (or implicitly generated) move operations are `noexcept`
  exactly when all members' and bases' moves are. You get it for free.
- A **hand-written** move constructor is *not* `noexcept` unless you say so.
- For templates, write a **conditional** `noexcept`:

```cpp
template <class T>
struct Box {
    Box(Box&& o) noexcept(std::is_nothrow_move_constructible_v<T>);
};
```

`noexcept(expr)` is also an operator: `noexcept(T(std::declval<T>()))` is a
compile-time bool. And `noexcept` is part of the function type since C++17.

Marking a function `noexcept` that does throw does not propagate the
exception: it calls `std::terminate`. Only promise what you keep.

### Implicit moves on `return`

```cpp
std::string f(std::string s) { return s; }            // moved (parameter)
std::string g() { std::string s = ...; return s; }    // NRVO, or moved
std::string h() { Pair p = ...; return p.first; }     // COPIED: a member is not "a local"
std::string k() { Pair p = ...; return std::move(p.first); } // moved
std::string bad() { std::string s; return std::move(s); }    // works, but disables NRVO
```

The rule: `return name;` where `name` is a local variable or by-value
parameter is treated as an rvalue first. Anything else (members, elements,
globals, references) is not.

## Your task

In `noexcept_moves.hpp` (namespace `idioms`):

1. `Message`: holds a `std::string topic` and `std::vector<int> payload`,
   and keeps a static counter of moves (it has hand-written move operations
   for that reason). Fix it so a `std::vector<Message>` moves on growth.
2. `Box<T>`: holds a `T` plus a `static inline int moves` counter (so again
   a hand-written move constructor). Its move constructor and move
   assignment must be `noexcept` **exactly when** `T`'s are.
3. `relocate(src, dst, n)`: move-construct (or copy-construct, per
   `std::move_if_noexcept`) `n` objects from `src` into raw storage at `dst`
   with `std::construct_at`, then destroy the sources. If a construction
   throws, destroy the ones already built in `dst` and rethrow (sources are
   intact, because in that case they were copied).
4. `take_last(v)`: given a `std::vector<T>` by value, return its last
   element without copying it.
5. `name_of(Pair p)`: return `p.first` without copying it.

The tests use types whose copy and move constructors count, and one whose
move constructor may throw.

<details><summary>Hints</summary>

- `relocate`: loop with an index `i`; in `catch (...)`, `std::destroy_n(dst, i)`
  then `throw;`.
- `Box`: `noexcept(std::is_nothrow_move_assignable_v<T>)` for the assignment.
</details>
