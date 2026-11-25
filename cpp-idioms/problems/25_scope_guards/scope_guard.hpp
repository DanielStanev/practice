// Problem 25: RAII scope guards.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace idioms {

template <class F>
class ScopeExit {
public:
    template <class Fn>
    explicit ScopeExit(Fn&& fn) : fn_(std::forward<Fn>(fn)) {}
    // TODO: destructor, no copies, move constructor, release()
    void release() noexcept {}

private:
    F fn_;
};

template <class F>
class ScopeFail {
public:
    template <class Fn>
    explicit ScopeFail(Fn&& fn) : fn_(std::forward<Fn>(fn)) {}
    // TODO
    void release() noexcept {}

private:
    F fn_;
};

template <class F>
class ScopeSuccess {
public:
    template <class Fn>
    explicit ScopeSuccess(Fn&& fn) : fn_(std::forward<Fn>(fn)) {}
    // TODO
    void release() noexcept {}

private:
    F fn_;
};

// Deduction guides (given): `ScopeExit g([&] { ... });` deduces F as the
// lambda's type, stored by value.
template <class F>
ScopeExit(F) -> ScopeExit<F>;
template <class F>
ScopeFail(F) -> ScopeFail<F>;
template <class F>
ScopeSuccess(F) -> ScopeSuccess<F>;

struct Account {
    long balance;
};

inline void transfer(Account& a, Account& b, long amount, bool fail_after_debit) {
    (void)a;
    (void)b;
    (void)amount;
    (void)fail_after_debit;
    throw std::logic_error("TODO: transfer");
}

}  // namespace idioms
