#include "fixed_string.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include "testing.hpp"

using namespace idioms;

namespace {
template <auto V>
struct Const {
    static constexpr auto value = V;
};
template <class L>
concept ConstexprOk = requires { Const<L{}()>::value; };
template <class L, class E>
void check_constexpr_eq(L, const E& expected, const char* expr, const char* file, int line) {
    if constexpr (ConstexprOk<L>) {
        ::testing::check_eq(Const<L{}()>::value, expected, expr, "expected", file, line);
    } else {
        ::testing::fail(file, line, std::string("not a constant expression: ") + expr);
    }
}
#define CHECK_CONSTEXPR_EQ(expr, expected) \
    check_constexpr_eq([] { return (expr); }, (expected), #expr, __FILE__, __LINE__)

template <FixedString S>
constexpr std::string_view as_view() {
    return S.view();
}

constexpr int fill_and_sum() {
    StaticVector<int, 8> v;
    for (int i = 1; i <= 5; ++i) v.push_back(i * i);
    v.pop_back();
    int s = 0;
    for (int x : v) s += x;
    return s + static_cast<int>(v.size());
}
}  // namespace

TEST(fixed_string_basics) {
    CHECK_CONSTEXPR_EQ(FixedString("hello").size(), std::size_t{5});
    // (view() of a temporary cannot be a template argument, so compare inside the expression)
    CHECK_CONSTEXPR_EQ(FixedString("hello").view() == "hello", true);
    CHECK_CONSTEXPR_EQ(as_view<"nttp">() == "nttp", true);
    CHECK_TRAIT(std::is_same_v<decltype(FixedString("abc")), FixedString<4>>);
}

TEST(fields) {
    using Px = Field<"price", double>;
    using Qty = Field<"qty", long>;
    CHECK_EQ(Px::name, std::string_view("price"));
    CHECK_EQ(Qty::name, std::string_view("qty"));
    CHECK_EQ((Field<"a", int>::id), std::uint32_t{0xE40C292Cu});  // FNV-1a("a")
    CHECK_EQ((Field<"", int>::id), std::uint32_t{2166136261u});
    CHECK(Px::id != Qty::id);
    CHECK_TRAIT(std::is_same_v<Field<"px", int>, Field<"px", int>>);
    CHECK_TRAIT(!std::is_same_v<Field<"px", int>, Field<"py", int>>);
    Px p{1.5};
    CHECK_EQ(p.value, 1.5);
}

TEST(static_vector_constexpr) {
    CHECK_CONSTEXPR_EQ(fill_and_sum(), 1 + 4 + 9 + 16 + 4);
    // Throwing is not allowed during constant evaluation in C++20 (even if caught),
    // so overflow is only tested at run time.
    StaticVector<int, 1> small;
    small.push_back(1);
    CHECK_THROWS_AS(small.push_back(2), std::length_error);
    StaticVector<std::string, 3> rt;  // works at run time with non-literal types too
    rt.push_back("x");
    CHECK_EQ(rt.back(), std::string("x"));
    CHECK_THROWS_AS((rt.push_back("y"), rt.push_back("z"), rt.push_back("w")), std::length_error);
}

TEST(symbol_table) {
    CHECK_CONSTEXPR_EQ(symbol_index(make_symbol_table(std::array<std::string_view, 3>{"c", "a", "b"}), "a"), 0);
    CHECK_CONSTEXPR_EQ(symbol_index(make_symbol_table(std::array<std::string_view, 3>{"c", "a", "b"}), "c"), 2);
    CHECK_CONSTEXPR_EQ(symbol_index(make_symbol_table(std::array<std::string_view, 3>{"c", "a", "b"}), "d"), -1);
    auto dup = [] { return make_symbol_table(std::array<std::string_view, 2>{"x", "x"}); };
    CHECK_TRAIT(!ConstexprOk<decltype(dup)>);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
