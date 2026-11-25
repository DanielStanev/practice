# 22 · Perfect forwarding and `decltype(auto)`

| Group | Difficulty | Builds on |
|---|---|---|
| H. Perfect forwarding | ★★☆☆☆ | 19, 20 |

## The concept

A wrapper function (a factory, `emplace_back`, a logging or timing
wrapper) should pass its arguments on **exactly** as it received them:
lvalues as lvalues (so they are copied, or bound to references), rvalues as
rvalues (so they are moved). That is perfect forwarding.

### Forwarding references

In a function template, `T&&` where `T` is *deduced* is not an rvalue
reference. It is a **forwarding reference**, and deduction encodes the
argument's category in `T`:

| argument | `T` deduced as | parameter type `T&&` after collapsing |
|---|---|---|
| lvalue `std::string` | `std::string&` | `std::string&` |
| const lvalue | `const std::string&` | `const std::string&` |
| rvalue | `std::string` | `std::string&&` |

**Reference collapsing**: `& &`, `& &&`, `&& &` all collapse to `&`; only
`&& &&` gives `&&`. `std::forward<T>(x)` is `static_cast<T&&>(x)`: with
`T = std::string&` it yields an lvalue, with `T = std::string` an rvalue.

Not forwarding references: `std::vector<T>&&` (not a bare `T`),
`const T&&`, and `T&&` in a member function of a class template where `T`
is the *class's* parameter (it is already fixed, not deduced).
`auto&&` is a forwarding reference too: `std::forward<decltype(x)>(x)`.

### Variadic forwarding

```cpp
template <class T, class... Args>
std::unique_ptr<T> my_make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```

The `...` after the closing parenthesis expands the pattern
`std::forward<Args>(args)` once per argument.

### Forwarding the return value: `decltype(auto)`

A wrapper around a call must also return what the call returns. `auto`
strips references (a function returning `int&` would come back as a copy).
`decltype(auto)` deduces the exact type, references included:

```cpp
template <class F, class... Args>
decltype(auto) timed(F&& f, Args&&... args) {
    auto start = now();
    decltype(auto) result = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    record(now() - start);
    if constexpr (std::is_rvalue_reference_v<decltype(result)>) return std::move(result);
    else return result;     // careful: `return (result);` would return a reference to a local!
}
```

(If `f` returns `void`, you cannot declare `result`; handle that with
`if constexpr (std::is_void_v<std::invoke_result_t<F, Args...>>)`.)

## Your task

In `perfect_forwarding.hpp` (namespace `idioms`):

1. `make_box<T>(args...)`: `std::unique_ptr<T>` built by forwarding.
2. `emplace_twice(vec, args...)`: `vec.emplace_back(args...)` twice with
   the same arguments, then returns the size. The arguments must be
   **copied** for the first call and **forwarded** only for the second
   (moving them in the first call would leave nothing for the second).
3. `counted_call(counter, f, args...)`: increments `counter`, then calls
   `f(args...)` with perfect forwarding of both `f` and the arguments, and
   returns **exactly** what `f` returns: references stay references,
   `void` stays `void`, values are not copied an extra time.
4. `ForwardingProbe`: a quiz in the header. For each line, write the type `T` deduces to.

<details><summary>Hints</summary>

- `std::invoke_result_t<F, Args...>` (the stub's return type) is exactly
  `decltype(std::invoke(std::declval<F>(), std::declval<Args>()...))`, so it
  agrees with `decltype(auto)` on a returned call expression. The
  `decltype(auto)` form is shorter and also works for lambdas with deduced
  return types defined inline.
- `counted_call` can be one line: `return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);`
  with return type `decltype(auto)`. Returning a call expression directly
  forwards its category; you only need the gymnastics above when you store
  the result first.
</details>
