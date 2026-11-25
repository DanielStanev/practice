#include "small_function.hpp"

#include <array>
#include <atomic>
#include <cstdlib>
#include <functional>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <vector>

#include "alloc_counter.hpp"
#include "testing.hpp"

using hft::SmallFunction;


namespace {

struct Counted {
    static inline int alive = 0;
    int bonus;
    explicit Counted(int b) : bonus(b) { ++alive; }
    Counted(const Counted& o) : bonus(o.bonus) { ++alive; }
    Counted(Counted&& o) noexcept : bonus(o.bonus) { ++alive; }
    ~Counted() { --alive; }
    int operator()(int x) const { return x + bonus; }
};

// Small, but its move constructor may throw, so it must not be stored inline
// (moving a SmallFunction is noexcept).
struct ThrowingMove {
    int v = 1;
    ThrowingMove() = default;
    ThrowingMove(const ThrowingMove&) = default;
    ThrowingMove(ThrowingMove&& o) : v(o.v) {}
    int operator()() const { return v; }
};

int add(int a, int b) { return a + b; }

}  // namespace

TEST(empty_function) {
    SmallFunction<void()> f;
    CHECK_FALSE(static_cast<bool>(f));
    CHECK_THROWS_AS(f(), std::bad_function_call);
    SmallFunction<void()> g = nullptr;
    CHECK_FALSE(static_cast<bool>(g));
}

TEST(calls_lambda_and_returns) {
    SmallFunction<int(int, int)> f = [](int a, int b) { return a * b; };
    CHECK(static_cast<bool>(f));
    CHECK_EQ(f(6, 7), 42);
}

TEST(function_pointer) {
    SmallFunction<int(int, int)> f = &add;
    CHECK_EQ(f(2, 3), 5);
    CHECK(f.is_inline());
}

TEST(small_callable_is_inline_and_does_not_allocate) {
    int captured = 10;
    long before = alloc_counter::count();
    SmallFunction<int(int)> f = [captured](int x) { return x + captured; };
    CHECK_EQ(alloc_counter::count(), before);
    CHECK(f.is_inline());
    CHECK_EQ(f(5), 15);
}

TEST(large_callable_goes_to_heap) {
    std::array<long, 16> big{};
    big[15] = 99;
    long before = alloc_counter::count();
    SmallFunction<long()> f = [big] { return big[15]; };
    CHECK_EQ(alloc_counter::count() - before, 1L);
    CHECK_FALSE(f.is_inline());
    CHECK_EQ(f(), 99L);
}

TEST(custom_buffer_size) {
    std::array<long, 16> big{};
    SmallFunction<long(), 128> f = [big] { return big[0] + 1; };
    CHECK(f.is_inline());
    CHECK_EQ(f(), 1L);
}

TEST(throwing_move_is_not_inline) {
    SmallFunction<int()> f = ThrowingMove{};
    CHECK_FALSE(f.is_inline());
    CHECK_EQ(f(), 1);
}

TEST(size_is_bounded) {
    // Buffer plus at most two pointers of bookkeeping.
    CHECK(sizeof(SmallFunction<void()>) <= 32 + 2 * sizeof(void*));
    CHECK(sizeof(SmallFunction<void(), 64>) <= 64 + 2 * sizeof(void*));
}

TEST(move_only_callable) {
    auto p = std::make_unique<int>(41);
    SmallFunction<int()> f = [p = std::move(p)] { return *p + 1; };
    CHECK_EQ(f(), 42);
    CHECK_TRAIT(!std::is_copy_constructible_v<SmallFunction<int()>>);
}

TEST(moves_transfer_target) {
    Counted::alive = 0;
    {
        SmallFunction<int(int)> a = Counted(3);
        CHECK_EQ(Counted::alive, 1);
        SmallFunction<int(int)> b = std::move(a);
        CHECK_FALSE(static_cast<bool>(a));
        CHECK_EQ(Counted::alive, 1);
        CHECK_EQ(b(1), 4);
        SmallFunction<int(int)> c;
        c = std::move(b);
        CHECK_EQ(c(2), 5);
        CHECK_EQ(Counted::alive, 1);
    }
    CHECK_EQ(Counted::alive, 0);
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<SmallFunction<int(int)>>);
    CHECK_TRAIT(std::is_nothrow_move_assignable_v<SmallFunction<int(int)>>);
}

TEST(heap_move_does_not_reallocate) {
    std::array<long, 16> big{};
    SmallFunction<long()> a = [big] { return big[0] + 7; };
    long before = alloc_counter::count();
    SmallFunction<long()> b = std::move(a);
    CHECK_EQ(alloc_counter::count(), before);
    CHECK_EQ(b(), 7L);
}

TEST(assignment_destroys_previous_target) {
    Counted::alive = 0;
    SmallFunction<int(int)> f = Counted(1);
    f = SmallFunction<int(int)>(Counted(2));
    CHECK_EQ(Counted::alive, 1);
    CHECK_EQ(f(0), 2);
    f = nullptr;
    CHECK_EQ(Counted::alive, 0);
    f = Counted(5);
    f.reset();
    CHECK_EQ(Counted::alive, 0);
}

TEST(stateful_mutable_lambda) {
    SmallFunction<int()> counter = [n = 0]() mutable { return ++n; };
    counter();
    counter();
    CHECK_EQ(counter(), 3);
}

TEST(reference_and_move_only_arguments) {
    SmallFunction<void(std::string&)> append = [](std::string& s) { s += "!"; };
    std::string s = "hi";
    append(s);
    CHECK_EQ(s, std::string("hi!"));

    SmallFunction<int(std::unique_ptr<int>)> take = [](std::unique_ptr<int> p) { return *p; };
    CHECK_EQ(take(std::make_unique<int>(8)), 8);
}

TEST(rejects_incompatible_callables) {
    auto wrong_args = [](std::string) { return 1; };
    CHECK_TRAIT(!std::is_constructible_v<SmallFunction<int(int)>, decltype(wrong_args)>);
    auto returns_string = [] { return std::string(); };
    CHECK_TRAIT(!std::is_constructible_v<SmallFunction<int()>, decltype(returns_string)>);
    // Return value may be discarded for void signatures.
    auto returns_int = [] { return 1; };
    CHECK_TRAIT(std::is_constructible_v<SmallFunction<void()>, decltype(returns_int)>);
}

TEST(vector_of_callbacks) {
    std::vector<SmallFunction<int(int)>> handlers;
    for (int i = 0; i < 100; ++i) handlers.emplace_back([i](int x) { return x + i; });
    int sum = 0;
    for (auto& h : handlers) sum += h(1);
    CHECK_EQ(sum, 100 + 4950);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
