# 30 · Which special members does the compiler give you?

| Group | Difficulty | Builds on |
|---|---|---|
| J. Rule of 0/3/5 | ★★★☆☆ | 20, 28, 29 |

## The concept

The compiler implicitly declares six special members: default
constructor, copy constructor, copy assignment, move constructor, move
assignment, destructor. Whether each one is declared, defaulted, or
deleted depends on what *you* declared. This is Howard Hinnant's table,
and it is worth memorising:

| You declare ↓ / compiler gives → | default ctor | dtor | copy ctor | copy assign | move ctor | move assign |
|---|---|---|---|---|---|---|
| nothing | defaulted | defaulted | defaulted | defaulted | defaulted | defaulted |
| any constructor | **not declared** | defaulted | defaulted | defaulted | defaulted | defaulted |
| default ctor | yours | defaulted | defaulted | defaulted | defaulted | defaulted |
| destructor | defaulted | yours | defaulted ⚠ | defaulted ⚠ | **not declared** | **not declared** |
| copy ctor | **not declared** | defaulted | yours | defaulted ⚠ | **not declared** | **not declared** |
| copy assign | defaulted | defaulted | defaulted ⚠ | yours | **not declared** | **not declared** |
| move ctor | **not declared** | defaulted | **deleted** | **deleted** | yours | **not declared** |
| move assign | defaulted | defaulted | **deleted** | **deleted** | **not declared** | yours |

⚠ = deprecated, still generated.

"Not declared" for the move operations is the sneaky one: the class is
still `std::is_move_constructible`, because an rvalue happily binds to
`const T&` and **the copy constructor is used**. Nothing fails to compile;
it is just slower.

Members also matter. A defaulted special member is **deleted** if it would
have to use something that cannot be used:

- a `const` or reference member: copy/move **assignment** is deleted (you
  cannot reassign them), construction still works;
- a member or base whose copy constructor is deleted (e.g. a
  `std::unique_ptr`): your copy constructor is deleted;
- a member without a default constructor: your default constructor is deleted.

And `= default` is not the same as `{}`: `T() = default;` keeps the type
trivial / an aggregate where possible; `T() {}` makes it user-provided
(non-trivial).

## Your task

`special_members.hpp` contains eleven empty structs. Give each one
members and/or special member declarations so that it has exactly the
properties in the comment above it. Use the table and the member rules;
avoid `= delete` except where the comment allows it (the point is to make
the compiler do it).

Every type also has the field `Tracker t;` already: `Tracker` counts its
copies and moves, so the tests can see which constructor actually ran.

<details><summary>Hints</summary>

- `CopiesWhenMoved`: what happens to the implicit move constructor if you
  declare a destructor?
- `AssignDeleted`: one `const` member.
- `Trivial`: plain data, nothing user-provided.
</details>
