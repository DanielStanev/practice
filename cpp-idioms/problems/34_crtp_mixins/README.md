# 34 · CRTP mixins: inheriting behaviour without virtual calls

| Group | Difficulty | Builds on |
|---|---|---|
| L. CRTP | ★★★☆☆ | 29, 32 |

## The concept

The **Curiously Recurring Template Pattern**: a class derives from a base
template instantiated with *itself*:

```cpp
template <class Derived>
struct Base {
    Derived&       derived()       { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }
    void hello() const { derived().name(); }   // calls Derived::name, resolved at compile time
};
struct Widget : Base<Widget> { void name() const; };
```

Because the base knows the derived type at compile time, it can call
derived members directly: no virtual table, no indirect call, fully
inlinable. Two big uses:

1. **Mixins**: inject a bundle of functionality (operators, counters,
   cloning, printing) into many unrelated classes from one implementation.
2. **Static polymorphism**: a framework base calls "hooks" in the derived
   class (problem 35).

### Mixin examples

```cpp
template <class D>
struct LessThanComparable {           // write <, get the rest
    friend bool operator>(const D& a, const D& b)  { return b < a; }
    friend bool operator<=(const D& a, const D& b) { return !(b < a); }
    ...
};
```

The operators are **hidden friends**: defined inside the class body, only
found by argument-dependent lookup when a `D` is involved. That keeps
overload sets small (faster compiles, better errors). In C++20 most
comparison mixins are replaced by `= default` `operator<=>`, but the
technique is the same for any family of operations.

**Per-type static state**: each `InstanceCounter<Order>` and
`InstanceCounter<Fill>` is a different class, so each has its own static
counters.

**Covariant cloning**: the classic `virtual clone()` must be written in
every derived class. A CRTP layer between the interface and the concrete
class writes it once:

```cpp
template <class Base, class Derived>
struct Cloneable : Base {
    std::unique_ptr<Base> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};
struct Stock : Cloneable<Instrument, Stock> { ... };
```

### Safety

`static_cast<Derived&>(*this)` is only valid if `*this` really is a
`Derived`. A typo like `struct Fill : Base<Order>` compiles and is UB. A
cheap guard: make the base's constructor private and befriend `Derived`,
so only the right class can derive from it.

## Your task

In `crtp_mixins.hpp` (namespace `idioms`):

1. `LessThanComparable<D>`: given `D`'s `operator<`, provide `>`, `<=`,
   `>=`, `==` and `!=` as hidden friends (equivalence: neither is less).
   Make the constructor private with `friend D;` so `struct X :
   LessThanComparable<Y>` does not compile.
2. `InstanceCounter<D>`: static `created()` and `alive()` per `D`,
   counting default, copy and move construction, and destruction.
3. `Cloneable<Base, Derived>`: implements `Base`'s pure virtual
   `std::unique_ptr<Base> clone() const`.
4. `Printable<D>`: `std::string to_string() const` producing
   `"<name>{<fields>}"` by calling `D::name()` (a static function returning a
   string view) and `D::fields(std::ostream&) const`.

<details><summary>Hints</summary>

- `InstanceCounter` needs user-written copy/move constructors (to count),
  a destructor, and `static inline` counters.
- In `Printable::to_string`, use `derived().fields(os)` and `D::name()`.
</details>
