# 23 · `std::invoke`, member pointers and `bind_front`

| Group | Difficulty | Builds on |
|---|---|---|
| H. Perfect forwarding | ★★★☆☆ | 22 |

## The concept

Generic code that takes "something callable" must handle more than
functions and lambdas. **Pointers to members** are callables too, with
their own syntax:

```cpp
struct Order { int qty; int notional(int px) const { return qty * px; } };

int Order::*        pq = &Order::qty;        // pointer to data member
int (Order::*pn)(int) const = &Order::notional;   // pointer to member function

Order o{10};  Order* op = &o;
o.*pq;               // 10            op->*pq;          // 10
(o.*pn)(5);          // 50            (op->*pn)(5);     // 50   (the parentheses are required)
```

`std::invoke(f, args...)` unifies all of it (the "INVOKE" rules):

- `f` is a pointer to member function, first arg is the object: `(obj.*f)(rest...)`;
  if the first arg is a pointer (or smart pointer): `((*ptr).*f)(rest...)`;
  if it is a `std::reference_wrapper`: `(ref.get().*f)(rest...)`.
- `f` is a pointer to data member, one arg: `obj.*f` (same three object forms).
  The result keeps the object's value category: `std::invoke(&Order::qty, std::move(o))` is an `int&&`.
- otherwise: `f(args...)`.

That is why projections (problem 18) accept `&Order::qty`: the algorithms
call them through `std::invoke`.

### `bind_front` and capturing packs

`std::bind_front(f, a, b)` returns a callable that calls `f(a, b, rest...)`.
It stores *copies* (decayed) of `f`, `a`, `b`, and forwards the remaining
arguments. C++20 lets lambdas capture a parameter pack by moving/forwarding
it in, which makes writing your own a few lines:

```cpp
template <class F, class... Bound>
auto my_bind_front(F&& f, Bound&&... bound) {
    return [f = std::forward<F>(f), ...b = std::forward<Bound>(bound)]
           (auto&&... rest) mutable -> decltype(auto) {
        return std::invoke(f, b..., std::forward<decltype(rest)>(rest)...);
    };
}
```

`std::bind_front` is preferred over the old `std::bind` (placeholders,
surprising copies and nested-bind behaviour) and over hand-written
capturing lambdas when you need to forward unknown arguments.

## Your task

In `invoke_and_bind.hpp` (namespace `idioms`):

1. `my_invoke(f, args...)`: your own `std::invoke` for the cases above:
   member function pointers and data member pointers called on an object,
   a pointer (raw or smart; anything you can dereference with `*`), or a
   `std::reference_wrapper`; and everything else as `f(args...)`. Return
   type `decltype(auto)`, perfect forwarding everywhere. Do not call
   `std::invoke` (or `std::apply`/`std::bind*`).
2. `my_bind_front(f, bound...)`: as above, but call `my_invoke` (yours).
3. `sum_by(orders, proj)`: sum `my_invoke(proj, order)` over a vector,
   where `proj` can be a lambda, a data member pointer or a member function
   pointer.

<details><summary>Hints</summary>

- Detect member pointers with `std::is_member_function_pointer_v<std::decay_t<F>>`
  and `std::is_member_object_pointer_v<std::decay_t<F>>`.
- Detect `reference_wrapper` with a small trait (`is_reference_wrapper<T>`
  specialised for `std::reference_wrapper<U>`).
- Detect "the object itself" with
  `std::is_base_of_v<Class, std::decay_t<Obj>>`, where `Class` is the class
  of the member pointer. Get `Class` with a helper:
  `template <class M, class C> C class_of(M C::*);`
- For a data member on an rvalue object, `std::forward<Obj>(obj).*pm` keeps
  the xvalue category; `decltype(auto)` keeps the `&&`.
</details>
