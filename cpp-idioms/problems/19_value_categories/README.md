# 19 · Value categories: lvalues, prvalues, xvalues

| Group | Difficulty | Builds on |
|---|---|---|
| G. Value categories and moves | ★★☆☆☆ | nothing |

## The concept

Every C++ **expression** (not variable!) has a type and a *value category*.
Move semantics, perfect forwarding, `decltype`, overload resolution and
lifetime extension are all defined in terms of it, so you cannot reason
about any of them without it.

```
            expression
           /          \
      glvalue        rvalue
     /       \      /      \
 lvalue       xvalue       prvalue
```

| Category | Meaning | Examples |
|---|---|---|
| **lvalue** | has identity, cannot be implicitly moved from | `x`, `*p`, `a[i]`, `++i`, `"literal"`, `std::cout << 1`, a call returning `T&`, **a named rvalue reference `r`** |
| **xvalue** ("eXpiring") | has identity, *may* be moved from | `std::move(x)`, a call returning `T&&`, `std::move(s).member`, `static_cast<T&&>(x)` |
| **prvalue** ("pure") | no identity yet, initialises a result object | `42`, `i++`, `a + b`, `T{}`, a call returning `T` by value, a lambda expression, `this` |

The single most important line in that table: **a variable declared as
`T&& r` is an lvalue when you name it.** The `&&` says what it may *bind to*,
not what the expression `r` is. That is why you must write `std::move(r)`
or `std::forward<T>(r)` to pass it on as an rvalue.

### Asking the compiler

`decltype` has two modes. `decltype(name)` gives the declared type of the
entity. `decltype((expression))` (double parentheses, or any non-name
expression) encodes the value category in the type:

| category of `e` | `decltype((e))` |
|---|---|
| lvalue | `T&` |
| xvalue | `T&&` |
| prvalue | `T` |

### Overload resolution by category

```cpp
void f(std::string&);         // #1 binds non-const lvalues only
void f(const std::string&);   // #2 binds anything (the fallback)
void f(std::string&&);        // #3 binds rvalues (xvalues and prvalues), preferred over #2 for them
```

`std::move` is nothing but `static_cast<T&&>(x)`: it moves nothing, it just
turns an lvalue expression into an xvalue so #3 gets picked.

## Your task

In `value_categories.hpp` (namespace `idioms`):

1. `category_of<T>()` returning `Category::lvalue`, `xvalue` or `prvalue`
   for `T = decltype((expr))`. The macro `CATEGORY(expr)` is provided.
2. The quiz: replace each `Category::unknown` constant `q01`..`q14` with
   the category of the expression in the comment next to it. Work them out
   **by reasoning first**; the tests compare your answers with what
   `CATEGORY` says. (Getting one wrong is the point; understand why.)
3. Three overloads `which(std::string&)`, `which(const std::string&)`,
   `which(std::string&&)` returning `"lvalue"`, `"const lvalue"` and
   `"rvalue"`.
4. `relay(x)`: a function template taking a forwarding reference `T&& x` that
   returns `which(...)` **as if the caller had called `which` directly**:
   `relay(s)` gives `"lvalue"`, `relay(std::move(s))` gives `"rvalue"`,
   `relay(std::as_const(s))` gives `"const lvalue"`. (A preview of problem 22.)

<details><summary>Hints</summary>

- `category_of`: `std::is_lvalue_reference_v<T>`, `std::is_rvalue_reference_v<T>`.
- `relay`: `which(x)` always picks the lvalue overload, because `x` is a
  name. `std::forward<T>(x)` restores the original category.
</details>
