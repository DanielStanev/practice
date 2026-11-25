# 40 · Object lifetime: dangling references and how to prevent them

| Group | Difficulty | Builds on |
|---|---|---|
| O. Lifetime and undefined behaviour | ★★★☆☆ | 18, 19, 38 |

## The concept

Every object has a **lifetime**: it starts when construction completes and
ends when the destructor starts (or the storage is reused or freed). A
reference, pointer, iterator or view that outlives its object **dangles**.
Reading through it is undefined behaviour: it may work in tests, return
garbage in production, or be "optimised" into something surprising.

### Temporaries die at the end of the full expression

```cpp
std::string make();
const char* p = make().c_str();             // dangles at the ';'
std::string_view sv = make();               // dangles at the ';'
auto& first = make_vector().front();        // dangles
for (char c : make_book().name()) ...       // (pre-C++23) dangles if name() returns a reference into the book
```

### Lifetime extension, and its limits

Binding a temporary **directly** to a `const T&` or `T&&` local extends the
temporary's life to the reference's scope:

```cpp
const std::string& s = make();              // OK: extended
```

It does **not** go through function calls:

```cpp
const std::string& a = std::max(make(), make());   // dangles: max returns a reference to a parameter
const int& m = std::min(1, 2);                      // dangles too
std::string_view v = std::string("x");              // dangles: string_view is not a reference
```

and it does not apply to a member reference initialised in a constructor.

### Defensive API design

You cannot add runtime checks for lifetime, but you can make the dangerous
call **not compile**:

```cpp
std::string_view first_word(const std::string& s);   // fine for lvalues...
std::string_view first_word(std::string&&) = delete;  // ...rvalues: rejected
```

Other tools: return by value when in doubt (moves are cheap); take
`std::string_view` parameters but never *store* them unless the owner is
documented to outlive you; ref-qualify accessors (`const T& get() const&`
plus `T get() &&`, problem 20) so calling an accessor on a temporary
returns a value, not a reference into the dying object.

### Tools

ASan finds use-after-free and use-after-return (`detect_stack_use_after_return=1`).
GCC 13+ has `-Wdangling-reference`; Clang has `-Wdangling` and
`[[clang::lifetimebound]]`.

## Your task

In `lifetime.hpp` (namespace `idioms`):

1. `first_word(s)`: `std::string_view` of the first space-separated word
   of a `std::string`. Calling it with an rvalue `std::string` must not
   compile. (`const char*` arguments should still work: a string literal
   lives forever. Make sure a literal does not get turned into a temporary
   `std::string` and rejected, or worse, accepted.)
2. `Instrument`: a class with a `std::string symbol_` and accessors
   `symbol()` that return a `const std::string&` on lvalues and a
   `std::string` (moved out) on rvalues, so
   `for (char c : make_instrument().symbol())` is safe.
3. `Tag`: stores a name given as `std::string_view` in its constructor, and
   must still return it correctly after the caller's string is destroyed.
   The stub shows the buggy version (storing the view) in a comment.
4. `larger(a, b)`: returns the larger of two `std::string`s. Make it
   impossible to dangle even when called with temporaries, while still not
   copying when called with lvalues... trick question: pick a design and
   defend it. The tests only check that the result of a call with two
   temporaries is still valid on the next line.
5. `make_adders()`: returns a `std::vector<std::function<int(int)>>`; the
   buggy version in the stub's comment captures a local by reference.
   Write a correct one.

The tests run with `ASAN_OPTIONS=detect_stack_use_after_return=1` added to
the defaults (see `run_tests.sh`).

<details><summary>Hints</summary>

- `first_word`: provide overloads for `const std::string&` and
  `const char*`, and `= delete` the `std::string&&` one.
- `Instrument::symbol() &&` must be a non-const member returning by value
  with `std::move(symbol_)`.
</details>
