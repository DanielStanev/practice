# 20 · Move semantics: stealing, sinks and ref-qualifiers

| Group | Difficulty | Builds on |
|---|---|---|
| G. Value categories and moves | ★★☆☆☆ | 19 |

## The concept

A **move** is a copy that is allowed to plunder its source. For a type that
owns a heap buffer, copying means allocate + copy bytes; moving means copy
one pointer and null out the source's. The move constructor and move
assignment are chosen by overload resolution whenever the source is an
rvalue (problem 19).

```cpp
class Buffer {
    std::size_t size_ = 0;
    char* data_ = nullptr;
public:
    Buffer(Buffer&& o) noexcept
        : size_(std::exchange(o.size_, 0)), data_(std::exchange(o.data_, nullptr)) {}
    Buffer& operator=(Buffer&& o) noexcept {
        if (this != &o) {
            delete[] data_;
            size_ = std::exchange(o.size_, 0);
            data_ = std::exchange(o.data_, nullptr);
        }
        return *this;
    }
};
```

`std::exchange(obj, new_value)` sets `obj` and returns the old value: the
idiomatic way to steal and reset in one expression.

**The moved-from state.** After a move, the source must still be *valid*
(destructible, assignable, and every operation without preconditions
works). The standard library only promises "valid but unspecified" for
its own types. For your own types, document it; "empty" is the usual choice.

### Sink parameters: take by value, then move

When a function stores its argument, take it **by value** and move it into
place:

```cpp
void set_name(std::string name) { name_ = std::move(name); }
set_name(s);              // 1 copy (into the parameter) + 1 move
set_name(std::move(s));   // 0 copies, 2 moves
set_name("literal");      // constructs the parameter directly, 1 move
```

One function covers both cases with at most one extra (cheap) move,
instead of writing `const T&` and `T&&` overloads.

### Ref-qualified member functions

A member function can be overloaded on the value category of `*this`:

```cpp
class Holder {
    std::string v_;
public:
    const std::string& value() const& { return v_; }          // on lvalues: no copy
    std::string        value() &&     { return std::move(v_); } // on rvalues: steal
};
std::string a = h.value();               // copies (h stays usable)
std::string b = std::move(h).value();    // moves out
std::string c = make_holder().value();   // moves out of the temporary
```

This is how `std::optional::value()` and `std::move(opt).value()` avoid
copies.

### `std::move` on a `const` object copies

`std::move(const_obj)` gives `const T&&`, which cannot bind to `T&&`, so
the **copy** constructor is chosen. No error, no warning, just a silent
copy. `const` locals cannot be moved from at the end of a function.

## Your task

In `move_semantics.hpp` (namespace `idioms`):

1. `Buffer`: owns `size` bytes allocated with `new char[size]` (zeroed).
   Rule of five with deep copy, `noexcept` stealing moves (moved-from:
   `size() == 0`, `data() == nullptr`), self-assignment safe for both copy
   and move, `noexcept` member `swap` and a free `swap`. The test counts
   `new[]` calls.
2. `Holder<T>`: `explicit Holder(T)` and `void set(T)` as sink
   parameters, `const T& get() const&` and `T get() &&` (steals).
3. `append(std::vector<T> base, const std::vector<T>& extra)`: returns
   `base` with `extra` appended. Taking `base` by value lets callers
   `std::move` their vector in and reuse its buffer: when they do, no element
   of `base` may be copied.

<details><summary>Hints</summary>

- Copy assignment with the strong guarantee: copy-and-swap
  (`Buffer tmp(other); swap(tmp); return *this;`).
- In `append`, `return base;` already moves (a by-value parameter is
  implicitly moved on return). Do not write `return std::move(base);`.
</details>
