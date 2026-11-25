# 04 · Writing a forward iterator (and its const twin)

| Group | Difficulty | Builds on |
|---|---|---|
| B. Writing iterators | ★★☆☆☆ | 01, 02 |

## The concept

To plug your own container into range-for, `<algorithm>` and `<ranges>`,
you give it `begin()`/`end()` returning iterator objects. A C++20
`std::forward_iterator` needs:

| Requirement | Why |
|---|---|
| `using value_type = ...;` | what you get when you copy an element (`std::iter_value_t`) |
| `using difference_type = std::ptrdiff_t;` | type of `distance` (signed) |
| `using iterator_category = std::forward_iterator_tag;` | so legacy algorithms (tag dispatch, problem 01) see it |
| `reference operator*() const` | note: **`const`** member, returns `T&` or a value |
| `It& operator++()` | prefix: step, return `*this` |
| `It operator++(int)` | postfix: copy, step, return the copy |
| `bool operator==(const It&) const` | `!=` is generated in C++20 |
| default constructible, copyable | forward iterators are regular values |

`std::forward_iterator<It>` is checked *structurally*: if one member type
or operator is missing or has the wrong signature, the concept is false and
`std::ranges::find(range, x)` refuses to compile. A `static_assert` right
after your class is the cheapest test you can write:

```cpp
static_assert(std::forward_iterator<MyList::iterator>);
```

`operator*` does not have to return a reference. An iterator that
*computes* values (a counting range, a generator) returns by value; its
`reference` type is then `value_type` itself. That is legal for C++20
`forward_iterator` (it was not for the old `LegacyForwardIterator`, which is
why such iterators report `input_iterator_tag` as their `iterator_category`
and `forward_iterator_tag` as their **`iterator_concept`**).

### `iterator` and `const_iterator` without writing it twice

A container needs both: `iterator` (yields `T&`) and `const_iterator`
(yields `const T&`), and an `iterator` must convert to a `const_iterator`
but not the other way. The standard trick is one template:

```cpp
template <bool Const>
class Iter {
    using Node = std::conditional_t<Const, const ListNode, ListNode>;
    Node* node_;
public:
    using value_type = T;
    using reference = std::conditional_t<Const, const T&, T&>;
    Iter() = default;
    // iterator -> const_iterator, enabled only when this is the const one.
    template <bool C = Const> requires C
    Iter(const Iter<false>& other) : node_(other.node_) {}
    friend class Iter<!Const>;   // so the converting constructor can read node_
};
using iterator = Iter<false>;
using const_iterator = Iter<true>;
```

**Pitfall.** The tempting one-liner
`Iter(const Iter<false>& o) requires Const` is *not* a template. Inside
`Iter<false>` its signature is `Iter(const Iter&)`: a user-declared copy
constructor (constrained away, but still declared), which suppresses the
implicit move constructor, so `Iter<false>` stops being movable and
`std::forward_iterator` silently becomes false. Making it a constructor
template avoids that, because a template is never a copy constructor.

## Your task

Implement two things in `forward_iterator.hpp` (namespace `idioms`).

### 1. `StepRange`

`StepRange(first, last, step)` yields `first, first+step, ...` while the
value is `< last` (`step > 0`). `StepRange(0, 10, 3)` yields `0 3 6 9`.
Its iterator computes values (returns `long` by value), stores no
container, and must satisfy `std::forward_iterator`.
`StepRange(5, 5, 1)` and `StepRange(7, 3, 1)` are empty.

### 2. `SList<T>`

A singly linked list with `push_front`, `size`, `begin`/`end` (const and
non-const), `cbegin`/`cend`, and `insert_after(const_iterator pos, T value)`
returning an `iterator` to the new node. Implement the iterators with the
`Iter<bool Const>` pattern above.

Requirements checked by the tests:

- both iterator types satisfy `std::forward_iterator`;
- `iterator` converts to `const_iterator`; the reverse does **not** compile;
- writing through `iterator` works, through `const_iterator` does not;
- `std::find`, `std::ranges::find`, `std::count_if`, range-for all work;
- no leaks (ASan) and copying the list is disabled (you only need moves if
  you want them).

The stubs are deliberately incomplete: the member types and postfix
operators are missing. The tests that need them report failure (instead of
breaking the build) until you add them.

<details><summary>Hints</summary>

- `StepRange::end()` must compare equal to the iterator you reach by
  stepping. Precompute the first value `>= last` on the step grid:
  `first + ceil((last - first) / step) * step`.
- For `StepRange::iterator` set `using iterator_concept =
  std::forward_iterator_tag;` and `using iterator_category =
  std::input_iterator_tag;` (prvalue `operator*`).
- The `SList` destructor: walk and `delete`. Mark the copy operations
  `= delete`.
</details>
