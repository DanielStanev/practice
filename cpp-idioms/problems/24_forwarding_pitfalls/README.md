# 24 · Forwarding pitfalls: greedy constructors, double forwarding, proxies

| Group | Difficulty | Builds on |
|---|---|---|
| H. Perfect forwarding | ★★★☆☆ | 22, 23 |

## The concept

Forwarding references match *everything*, and that greed causes three
classic bugs.

### 1. The greedy constructor hijacks copying

```cpp
struct Named {
    std::string name;
    template <class S>
    explicit Named(S&& s) : name(std::forward<S>(s)) {}
};
Named a("x");
Named b(a);     // ERROR: tries to build a std::string from a Named&
```

For a **non-const** lvalue `a`, the template instantiates with
`S = Named&`, giving `Named(Named&)`: a better match than the copy
constructor `Named(const Named&)`, which needs a `const` conversion. The fix
is to constrain the template so it never accepts the class itself (or
anything derived from it):

```cpp
template <class S>
    requires (!std::is_base_of_v<Named, std::remove_cvref_t<S>>)
             && std::constructible_from<std::string, S>
explicit Named(S&& s);
```

The same thing bites overloading: `void log(int)` vs
`template <class T> void log(T&&)`. Calling `log(short_value)` or
`log(size_t_value)` picks the template (exact match beats a conversion).

### 2. Forwarding the same argument twice

```cpp
template <class T>
void notify_all(T&& msg) {
    for (auto& l : listeners) l(std::forward<T>(msg));   // first listener may steal msg
}
```

After the first call moves from `msg`, the rest get a moved-from object.
Forward (or move) **only on the last use**; pass lvalues before that.

### 3. `auto&` and proxy references

`std::vector<bool>` stores bits, so its `operator[]` and iterators return a
*proxy object* by value (`std::vector<bool>::reference`), not a `bool&`.

```cpp
for (auto& b : bits) b = true;   // ERROR: cannot bind a non-const lvalue ref to a temporary proxy
for (auto b : bits) b = true;    // compiles and works for vector<bool> (the proxy writes through)...
                                 // ...but silently does nothing for vector<int>!
for (auto&& b : bits) b = true;  // correct for both: binds to the proxy or to the int&
```

So in generic code that modifies elements, use `auto&&` (a forwarding
reference: it accepts lvalues and temporaries). Views with transform
(problem 16) also produce prvalues, the same reasoning applies.

## Your task

In `forwarding_pitfalls.hpp` (namespace `idioms`):

1. `Named`: a forwarding constructor from anything `std::string` can be
   constructed from, which must **not** hijack copy or move (including from
   derived classes). The stub's version is unconstrained, and because
   `Named` also converts to `std::string_view`, the hijack even compiles:
   `Named b(a)` silently goes through the template (an extra conversion and
   allocation) instead of the copy constructor. A counter in the template
   lets the tests see that. Constrain it.
2. `Logger`: `log(long long id)` records `"id:<n>"` and a template
   `log(T&&)` that records `"text:<s>"` for anything convertible to
   `std::string_view`. Integers of *any* integral type must go to the id
   overload. Constrain the template.
3. `broadcast(arg, fs...)`: call every `f` with `arg`; only the **last**
   call may receive it as an rvalue (forwarded); the others get an lvalue.
   Return the number of calls. With zero functions, do nothing.
4. `assign_all(range, value)`: set every element of any range (including
   `std::vector<bool>` and a `std::views::transform` over an array of
   pointers to `int` that returns `int&`) to `value`.

<details><summary>Hints</summary>

- `broadcast`: a fold over the comma operator on all but the last is awkward.
  Simpler: recurse. `broadcast(arg, f)` forwards; `broadcast(arg, f, rest...)`
  calls `f(arg)` and recurses with `std::forward<Arg>(arg)`.
- `Logger::log` constraint: `requires (!std::is_integral_v<std::remove_cvref_t<T>>)
  && std::convertible_to<T, std::string_view>`.
</details>
