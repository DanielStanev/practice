#include "scope_guard.hpp"

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
// Runs a ScopeFail inside a destructor that executes during unwinding of
// another exception. The inner scope itself exits normally, so the guard
// must NOT fire. (std::uncaught_exception() singular gets this wrong.)
struct CleansUpOnDestruction {
    int* fired;
    ~CleansUpOnDestruction() {
        ScopeFail g([this] { ++*fired; });
    }
};
}  // namespace

TEST(scope_exit_always) {
    int n = 0;
    {
        ScopeExit g([&] { ++n; });
        CHECK_EQ(n, 0);
    }
    CHECK_EQ(n, 1);
    try {
        ScopeExit g([&] { ++n; });
        throw std::runtime_error("x");
    } catch (const std::runtime_error&) {
    }
    CHECK_EQ(n, 2);
}

TEST(release_disarms) {
    int n = 0;
    {
        ScopeExit g([&] { ++n; });
        g.release();
    }
    CHECK_EQ(n, 0);
}

TEST(scope_fail_only_on_exception) {
    int n = 0;
    { ScopeFail g([&] { ++n; }); }
    CHECK_EQ(n, 0);
    try {
        ScopeFail g([&] { ++n; });
        throw 1;
    } catch (int) {
    }
    CHECK_EQ(n, 1);
}

TEST(scope_success_only_on_normal_exit) {
    int n = 0;
    { ScopeSuccess g([&] { ++n; }); }
    CHECK_EQ(n, 1);
    try {
        ScopeSuccess g([&] { ++n; });
        throw 1;
    } catch (int) {
    }
    CHECK_EQ(n, 1);
}

TEST(scope_fail_inside_unwinding_destructor) {
    int fired = 0;
    try {
        CleansUpOnDestruction c{&fired};
        throw std::runtime_error("outer");
    } catch (const std::runtime_error&) {
    }
    CHECK_EQ(fired, 0);
}

TEST(guards_move_not_copy) {
    auto f = [] {};
    CHECK_TRAIT(!std::is_copy_constructible_v<ScopeExit<decltype(f)>>);
    CHECK_TRAIT(std::is_move_constructible_v<ScopeExit<decltype(f)>>);
    CHECK_TRAIT(!std::is_copy_constructible_v<ScopeFail<decltype(f)>>);
    CHECK_TRAIT(std::is_move_constructible_v<ScopeSuccess<decltype(f)>>);
    int n = 0;
    {
        auto make = [&n] { return ScopeExit([&n] { ++n; }); };
        auto g = make();
        auto h = std::move(g);  // only h runs
    }
    CHECK_EQ(n, 1);
}

TEST(scope_success_can_throw) {
    CHECK_TRAIT(!std::is_nothrow_destructible_v<ScopeSuccess<void (*)()>>);
    bool caught = false;
    try {
        ScopeSuccess g([] { throw std::runtime_error("commit failed"); });
    } catch (const std::runtime_error&) {
        caught = true;
    }
    CHECK(caught);
}

TEST(transfer_is_all_or_nothing) {
    Account a{100}, b{0};
    transfer(a, b, 30, false);
    CHECK_EQ(a.balance, 70L);
    CHECK_EQ(b.balance, 30L);
    CHECK_THROWS_AS(transfer(a, b, 50, true), std::runtime_error);
    CHECK_EQ(a.balance, 70L);
    CHECK_EQ(b.balance, 30L);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
