# 03 · The standard iterator adaptors

| Group | Difficulty | Builds on |
|---|---|---|
| A. Iterator fundamentals | ★☆☆☆☆ | 01 |

## The concept

An *iterator adaptor* wraps something (a container, a stream, another
iterator) and presents it through the iterator interface, so any algorithm
can use it. Knowing them lets you write one-line solutions with standard
algorithms instead of hand loops, and they appear constantly in interview
code.

### Insert iterators (output iterators)

`std::copy(first, last, out)` *assigns* through `out`; it never grows a
container. Writing into `v.begin()` of an empty vector is a buffer overflow.
Insert iterators turn `*it = x` into a container call:

```cpp
std::vector<int> out;
std::copy(src.begin(), src.end(), std::back_inserter(out));   // out.push_back(x)
std::copy(src.begin(), src.end(), std::front_inserter(dq));   // dq.push_front(x): reverses order!
std::copy(src.begin(), src.end(), std::inserter(set, set.end())); // set.insert(hint, x)
```

### Stream iterators

```cpp
std::istringstream in("3 1 4");
std::vector<int> v{std::istream_iterator<int>(in), std::istream_iterator<int>()};
// default-constructed istream_iterator is the "end of stream" iterator.
// Reading stops at EOF *or* at the first token that fails to parse.

std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, ","));
// prints "3,1,4," -- the delimiter is written after every element.
```

### Reverse iterators and `base()`

`v.rbegin()` is a `std::reverse_iterator` wrapping `v.end()`. Dereferencing a
reverse iterator returns `*std::prev(base())`, so a reverse iterator and its
`base()` point at **neighbouring** elements:

```
 elements:     [ a ][ b ][ c ]
 r = find(rbegin, rend, b)  -> *r == b
 r.base()                    -> points at c
 std::next(r).base()         -> points at b   <- use this to erase *r
```

`v.erase(r.base())` is the classic off-by-one bug: it erases the wrong
element, or `end()` (UB) if `r == rbegin()`.

### Move iterators

`std::make_move_iterator(it)` makes `*it` return an rvalue reference, so
algorithms and constructors *move* instead of copying:

```cpp
dst.insert(dst.end(), std::make_move_iterator(src.begin()),
                      std::make_move_iterator(src.end()));
```

## Your task

Implement in `iterator_adaptors.hpp` (namespace `idioms`). Each body should
be a few lines built from the adaptors above and standard algorithms; no
raw `for` loops.

1. `parse_ints(text)`: whitespace-separated integers, stopping at the first
   token that is not an integer. `"1 2 x 3"` gives `{1, 2}`.
2. `join(values, sep)`: `{1,2,3}`, `", "` gives `"1, 2, 3"` (no trailing
   separator). Use `std::ostream_iterator` for all but the last element.
3. `erase_last(v, value)`: erase the **last** occurrence of `value`; return
   whether something was erased. Use reverse iterators and `base()`.
4. `prepend_all(dq, values)`: insert `values` at the front of the deque
   **keeping their order** (`dq = {9}`, `values = {1,2}` gives `{1,2,9}`).
   Think about what `front_inserter` does to the order.
5. `move_append(dst, src)`: append all elements of `src` to `dst` by
   moving (the tests count copies: there must be none), then `src.clear()`.

<details><summary>Hints</summary>

- `join`: copy `[first, prev(last))` with `ostream_iterator<int>(os, sep)`,
  then write the last element yourself.
- `prepend_all`: either copy from `values.rbegin()` with `front_inserter`,
  or use `dq.insert(dq.begin(), ...)`.
- `move_append`: call `dst.reserve` first, or the copies may come from
  `dst`'s own reallocation if the type's move constructor is not
  `noexcept`. (Ours is.)
</details>
