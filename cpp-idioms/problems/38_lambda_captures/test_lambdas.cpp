#include "lambdas.hpp"

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "testing.hpp"

using namespace idioms;

TEST(counter_has_state_per_copy) {
    auto c = make_counter(10);
    CHECK_EQ(c(), 10);
    CHECK_EQ(c(), 11);
    auto d = c;  // copies the state
    CHECK_EQ(d(), 12);
    CHECK_EQ(d(), 13);
    CHECK_EQ(c(), 12);  // c continues on its own
}

TEST(multipliers_capture_by_value) {
    auto ms = make_multipliers(5);
    CHECK_EQ(ms.size(), std::size_t{5});
    for (int i = 0; i < 5; ++i) CHECK_EQ(ms[i](10), 10 * i);
}

TEST(compose_order) {
    auto inc = [](int x) { return x + 1; };
    auto dbl = [](int x) { return x * 2; };
    auto sq = [](int x) { return x * x; };
    CHECK_EQ(compose(inc, dbl, sq)(3), 19);  // inc(dbl(sq(3)))
    CHECK_EQ(compose(sq, inc)(3), 16);
    CHECK_EQ(compose(dbl)(21), 42);
    auto to_str = [](int x) { return std::to_string(x); };
    CHECK_EQ(compose(to_str, dbl)(4), std::string("8"));
}

TEST(memoize_calls_once_per_argument) {
    int calls = 0;
    auto slow_square = [&calls](long x) {
        ++calls;
        return x * x;
    };
    auto fast = memoize(slow_square);
    CHECK_EQ(fast(12), 144L);
    CHECK_EQ(fast(12), 144L);
    CHECK_EQ(fast(3), 9L);
    CHECK_EQ(fast(12), 144L);
    CHECK_EQ(calls, 2);
}

TEST(owner_is_move_only) {
    auto f = make_owner(std::make_unique<int>(7));
    CHECK_EQ(f(), 7);
    CHECK_TRAIT(!std::is_copy_constructible_v<decltype(make_owner(nullptr))>);
    auto g = std::move(f);
    CHECK_EQ(g(), 7);
}

TEST(printer_outlives_order) {
    std::function<std::string()> p;
    {
        auto o = std::make_unique<Order>(Order{42, 101.5});
        p = o->printer();
    }  // the Order is gone; ASan would catch a [this] capture
    CHECK_EQ(p(), std::string("42@101.5"));
}

TEST(tick_size_values) {
    CHECK_EQ(tick_size("XNAS", 10.0), 0.01);
    CHECK_EQ(tick_size("XNAS", 0.5), 0.0001);
    CHECK_EQ(tick_size("XLON", 0.5), 0.5);
    CHECK_EQ(tick_size("XTKS", 3.0), 0.01);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
