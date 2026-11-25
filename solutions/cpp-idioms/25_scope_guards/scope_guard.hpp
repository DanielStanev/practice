// Reference solution for problem 25.
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
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit(ScopeExit&& o) noexcept(std::is_nothrow_move_constructible_v<F>)
        : fn_(std::move(o.fn_)), active_(std::exchange(o.active_, false)) {}
    ~ScopeExit() {
        if (active_) fn_();
    }
    void release() noexcept { active_ = false; }

private:
    F fn_;
    bool active_ = true;
};

template <class F>
class ScopeFail {
public:
    template <class Fn>
    explicit ScopeFail(Fn&& fn) : fn_(std::forward<Fn>(fn)) {}
    ScopeFail(const ScopeFail&) = delete;
    ScopeFail& operator=(const ScopeFail&) = delete;
    ScopeFail(ScopeFail&& o) noexcept(std::is_nothrow_move_constructible_v<F>)
        : fn_(std::move(o.fn_)), active_(std::exchange(o.active_, false)), exceptions_(o.exceptions_) {}
    ~ScopeFail() {
        if (active_ && std::uncaught_exceptions() > exceptions_) fn_();
    }
    void release() noexcept { active_ = false; }

private:
    F fn_;
    bool active_ = true;
    int exceptions_ = std::uncaught_exceptions();
};

template <class F>
class ScopeSuccess {
public:
    template <class Fn>
    explicit ScopeSuccess(Fn&& fn) : fn_(std::forward<Fn>(fn)) {}
    ScopeSuccess(const ScopeSuccess&) = delete;
    ScopeSuccess& operator=(const ScopeSuccess&) = delete;
    ScopeSuccess(ScopeSuccess&& o) noexcept(std::is_nothrow_move_constructible_v<F>)
        : fn_(std::move(o.fn_)), active_(std::exchange(o.active_, false)), exceptions_(o.exceptions_) {}
    ~ScopeSuccess() noexcept(noexcept(std::declval<F&>()())) {
        if (active_ && std::uncaught_exceptions() <= exceptions_) fn_();
    }
    void release() noexcept { active_ = false; }

private:
    F fn_;
    bool active_ = true;
    int exceptions_ = std::uncaught_exceptions();
};

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
    a.balance -= amount;
    ScopeFail undo([&] { a.balance += amount; });
    if (fail_after_debit) throw std::runtime_error("transfer failed");
    b.balance += amount;
}

}  // namespace idioms
