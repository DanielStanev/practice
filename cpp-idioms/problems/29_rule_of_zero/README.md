# 29 · The rule of zero, and value semantics for polymorphic members

| Group | Difficulty | Builds on |
|---|---|---|
| J. Rule of 0/3/5 | ★★★☆☆ | 26, 28 |

## The concept

Problem 28 showed how much care hand-written special members need. The
**rule of zero** says: don't write them. Put each resource in a member
whose type already manages it (`std::vector`, `std::string`,
`std::unique_ptr`, your `UniqueResource`), and the compiler-generated
copy, move and destructor of your class are automatically correct,
exception-safe, and `noexcept` where they should be. Only the small
resource-owning building blocks follow the rule of five; everything built
from them follows the rule of zero.

```cpp
struct Strategy {                          // rule of zero: nothing to write
    std::string name;
    std::vector<double> weights;
    std::unique_ptr<Model> model;          // makes Strategy move-only, which is honest
};
```

Watch out for the ways you lose the defaults:

- Declaring a destructor (even `~X() = default;`) suppresses the implicit
  **move** operations; copies are used instead (silently, and slower).
- Declaring any copy operation suppresses the implicit moves too.
- Declaring a move operation deletes the implicit copies.
- If you must declare one (e.g. a virtual destructor in a base class),
  `= default` all five explicitly.

### The polymorphic member problem

`std::unique_ptr<Base>` is move-only. If your class needs to be
**copyable** and holds a polymorphic object, the rule of zero seems to fail:
copying a `unique_ptr<Base>` would need to know the dynamic type.
The fix is a small rule-of-five building block that deep-copies through
the dynamic type, after which the outer class is rule-of-zero again. That
building block is a `ClonePtr<T>` (C++26 standardises the idea as
`std::polymorphic<T>`).

Two ways to know how to copy the dynamic type:

1. Require `virtual std::unique_ptr<Base> clone() const` in the hierarchy
   (intrusive, the classic approach).
2. **Type erasure**: when constructing from a `Derived`, remember a function
   pointer that knows how to copy a `Derived`. No changes to the
   hierarchy needed:

```cpp
template <class T>
class ClonePtr {
    T* ptr_;
    T* (*copier_)(const T*);       // set once, from the concrete type
public:
    template <class U, class... Args>
    static ClonePtr make(Args&&... args) {
        ClonePtr p;
        p.ptr_ = new U(std::forward<Args>(args)...);
        p.copier_ = [](const T* src) -> T* { return new U(*static_cast<const U*>(src)); };
        return p;
    }
    ...
};
```

(A capture-less lambda converts to a plain function pointer.)

## Your task

In `clone_ptr.hpp` (namespace `idioms`):

1. `ClonePtr<T>` using approach 2: `make<U>(args...)`, default constructor
   (empty), copy (deep, through the dynamic type), move (`noexcept`,
   source becomes empty), copy and move assignment, destructor, `get()`,
   `operator*`, `operator->`, `explicit operator bool`. `T` needs a virtual
   destructor; `static_assert` it.
2. `Portfolio`: holds a `std::string name` and a
   `std::vector<ClonePtr<Instrument>> legs` and has **no user-declared
   special member functions**. `add<U>(args...)` adds a leg; `value()` sums
   `legs[i]->value()`. Copying a portfolio deep-copies the legs.
3. Fix `LegacyBook` (in the stub): it declares a destructor "for logging"
   and so lost its moves. Make moving a `LegacyBook` move its
   `std::vector<std::string>` instead of copying it, while keeping the
   destructor.

<details><summary>Hints</summary>

- In `ClonePtr`, copy constructor: `ptr_(o.ptr_ ? o.copier_(o.ptr_) : nullptr), copier_(o.copier_)`.
- `make<U>` should `static_assert(std::is_base_of_v<T, U>)`.
- Assignment: copy-and-swap handles both forms.
- `LegacyBook`: add `= default` move and copy operations.
</details>
