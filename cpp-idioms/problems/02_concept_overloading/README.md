# 02 · Overloading generic algorithms on iterator concepts

| Group | Difficulty | Builds on |
|---|---|---|
| A. Iterator fundamentals | ★☆☆☆☆ | 01 |

## The concept

Problem 01 used tag dispatch to choose an implementation. C++20 lets you
write the choice directly in the overload set using **constrained
templates**:

```cpp
template <std::forward_iterator It>
It pick(It first, It last);           // #1

template <std::random_access_iterator It>
It pick(It first, It last);           // #2
```

For a `std::vector<int>::iterator`, both overloads are viable. The compiler
picks #2 because `std::random_access_iterator` **subsumes**
`std::forward_iterator`: its definition is built from the forward concept
plus more requirements, so it is "more constrained". This only works when
one concept is literally defined in terms of the other (as the standard
iterator concepts are). Two unrelated concepts that happen to overlap give
you an ambiguity error instead.

Constraints also reject bad calls early. If an algorithm needs `--it`,
writing `template <std::bidirectional_iterator It>` turns a 40-line template
error from deep inside your loop into "constraints not satisfied" at the
call site. You can test whether a call is allowed with a `requires`
expression:

```cpp
static_assert(!requires(std::forward_list<int>::iterator i) { is_palindrome(i, i); });
```

### Useful helpers

- `std::next(it, n)` / `std::prev(it, n)` return a moved copy (they use
  `std::advance` internally, so they are O(1) on random access).
- `std::ranges::next(it, bound)` steps until `bound`.
- The **two-pointer** trick: to find the k-th element from the end of a
  forward-only sequence in one pass, start a "lead" iterator k steps ahead,
  then move both until lead hits the end.
- The **tortoise and hare** trick: to find the middle in one pass, move one
  iterator by 1 and another by 2.

## Your task

Implement in `concept_overloading.hpp` (namespace `idioms`). Each function
must be a set of constrained overloads, not `if constexpr`.

1. `middle(first, last)`: iterator to element `distance/2` (for 5 elements,
   index 2; for 4, index 2).
   * forward iterators: single pass, tortoise and hare. Do not call
     `std::distance`: the tests count increments.
   * random access: O(1), zero increments.
2. `last_n(first, last, n)`: iterator to the first of the last `n` elements
   (or `first` if there are fewer than `n`).
   * forward: single pass, two pointers (lead walks `distance`, lag walks `distance - n`; no `std::distance`).
   * bidirectional: walk back from `last` (at most `n` decrements, no
     increments).
   * random access: O(1).
3. `is_palindrome(first, last)`: true if the sequence reads the same both
   ways. Must require `std::bidirectional_iterator`, so calling it with a
   `std::forward_list` iterator does not compile.

## Running the tests

```bash
./run_tests.sh
```

<details><summary>Hints</summary>

- In `middle` for forward iterators, step the hare twice per loop but check
  for `last` after *each* step.
- In `last_n` for bidirectional iterators, stop decrementing when you reach
  `first`.
- `is_palindrome`: move inwards from both ends and stop when the iterators
  meet or cross. With bidirectional iterators you cannot compare with `<`,
  so check `first == last` after each step.
</details>
