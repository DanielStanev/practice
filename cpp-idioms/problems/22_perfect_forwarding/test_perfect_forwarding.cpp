#include "perfect_forwarding.hpp"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
struct Tracked {
    static inline int copies = 0, moves = 0;
    std::string v;
    Tracked(std::string s) : v(std::move(s)) {}
    Tracked(const Tracked& o) : v(o.v) { ++copies; }
    Tracked(Tracked&& o) noexcept : v(std::move(o.v)) { ++moves; }
    static void reset() { copies = moves = 0; }
};

struct Pair {
    Tracked a, b;
    Pair(const Tracked& x, Tracked&& y) : a(x), b(std::move(y)) {}  // binds only an lvalue then an rvalue
};

struct Widget {
    std::string name;
    int n;
    Widget(std::string s, int k) : name(std::move(s)), n(k) {}
};

int global = 5;
int& get_global() { return global; }
std::unique_ptr<int> make_ptr(int v) { return std::make_unique<int>(v); }
void nothing() {}
}  // namespace

TEST(make_box_forwards) {
    Tracked t("lv");
    Tracked::reset();
    auto p = make_box<Pair>(t, Tracked("rv"));
    CHECK_EQ(Tracked::copies, 1);  // only the lvalue
    CHECK_EQ(p->a.v, std::string("lv"));
    CHECK_EQ(p->b.v, std::string("rv"));
    auto w = make_box<Widget>("abc", 3);
    CHECK_EQ(w->name, std::string("abc"));
    auto m = make_box<std::unique_ptr<int>>(std::make_unique<int>(9));  // move-only argument
    CHECK_EQ(**m, 9);
}

TEST(emplace_twice_copies_then_forwards) {
    std::vector<Tracked> v;
    v.reserve(4);
    Tracked::reset();
    std::size_t n = emplace_twice(v, Tracked("x"));
    CHECK_EQ(n, std::size_t{2});
    CHECK_EQ(Tracked::copies, 1);
    CHECK_EQ(Tracked::moves, 1);
    CHECK_EQ(v[0].v, std::string("x"));
    CHECK_EQ(v[1].v, std::string("x"));  // not a moved-from husk
    std::vector<Widget> w;
    emplace_twice(w, std::string("long enough to not be in the small buffer"), 1);
    CHECK_EQ(w[0].name, w[1].name);
}

TEST(counted_call_returns_exact_type) {
    int count = 0;
    CHECK_TRAIT(std::is_same_v<decltype(counted_call(count, get_global)), int&>);
    CHECK_TRAIT(std::is_same_v<decltype(counted_call(count, make_ptr, 1)), std::unique_ptr<int>>);
    CHECK_TRAIT(std::is_same_v<decltype(counted_call(count, nothing)), void>);
    int& r = counted_call(count, get_global);
    r = 42;
    CHECK_EQ(global, 42);
    auto p = counted_call(count, make_ptr, 7);
    CHECK_EQ(*p, 7);
    counted_call(count, nothing);
    CHECK_EQ(count, 3);
}

TEST(counted_call_forwards_arguments) {
    int count = 0;
    Tracked t("a");
    Tracked::reset();
    auto by_value = [](Tracked x) { return x.v.size(); };
    counted_call(count, by_value, t);
    CHECK_EQ(Tracked::copies, 1);
    Tracked::reset();
    counted_call(count, by_value, std::move(t));
    CHECK_EQ(Tracked::copies, 0);
    // a mutable lambda passed as an lvalue is called in place, not copied
    int calls = 0;
    auto counter = [&calls]() mutable { return ++calls; };
    counted_call(count, counter);
    counted_call(count, counter);
    CHECK_EQ(calls, 2);
}

TEST(quiz_deduction) {
    CHECK_TRAIT(std::is_same_v<quiz::T_for_g_s, std::string&>);
    CHECK_TRAIT(std::is_same_v<quiz::T_for_g_cs, const std::string&>);
    CHECK_TRAIT(std::is_same_v<quiz::T_for_g_move_s, std::string>);
    CHECK_TRAIT(std::is_same_v<quiz::T_for_g_ref, std::string&>);
    CHECK_TRAIT(std::is_same_v<quiz::T_for_g_literal, int>);
    CHECK_TRAIT(std::is_same_v<quiz::Param_for_g_s, std::string&>);
    CHECK_TRAIT(std::is_same_v<quiz::Param_for_g_move_s, std::string&&>);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
