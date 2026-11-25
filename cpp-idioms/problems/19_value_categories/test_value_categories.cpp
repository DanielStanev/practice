#include "value_categories.hpp"

#include <string>
#include <utility>

#include "testing.hpp"

using idioms::Category;

// Globals for the quiz (not in an anonymous namespace: make_s() returns S).
[[maybe_unused]] int i = 0;
[[maybe_unused]] int* p = &i;
[[maybe_unused]] int&& rr = 5;
[[maybe_unused]] std::string s;
struct S {
    int m;
};
[[maybe_unused]] S obj;

// Declared only: they are used in unevaluated decltype operands, never called.
int& get_ref();
int&& get_xref();
int get_val();
S make_s();

namespace testing {
template <>
inline std::string describe(const Category& c) {
    switch (c) {
        case Category::lvalue: return "lvalue";
        case Category::xvalue: return "xvalue";
        case Category::prvalue: return "prvalue";
        default: return "unknown";
    }
}
}  // namespace testing

TEST(category_of_basics) {
    int x = 0;
    CHECK_EQ(CATEGORY(x), Category::lvalue);
    CHECK_EQ(CATEGORY(std::move(x)), Category::xvalue);
    CHECK_EQ(CATEGORY(x * 2), Category::prvalue);
    CHECK_EQ(CATEGORY(std::string("tmp")), Category::prvalue);
}

#define QUIZ(q, expr) TEST(quiz_##q) { CHECK_EQ(idioms::q, CATEGORY(expr)); }
QUIZ(q01, i)
QUIZ(q02, i + 1)
QUIZ(q03, ++i)
QUIZ(q04, i++)
QUIZ(q05, *p)
QUIZ(q06, rr)
QUIZ(q07, std::move(i))
QUIZ(q08, "hello")
QUIZ(q09, get_ref())
QUIZ(q10, get_xref())
QUIZ(q11, get_val())
QUIZ(q12, make_s().m)
QUIZ(q13, std::move(obj).m)
QUIZ(q14, s = "x")

TEST(which_overloads) {
    std::string a = "a";
    const std::string b = "b";
    CHECK_EQ(idioms::which(a), std::string_view("lvalue"));
    CHECK_EQ(idioms::which(b), std::string_view("const lvalue"));
    CHECK_EQ(idioms::which(std::move(a)), std::string_view("rvalue"));
    CHECK_EQ(idioms::which(std::string("t")), std::string_view("rvalue"));
    std::string&& named = std::move(a);
    CHECK_EQ(idioms::which(named), std::string_view("lvalue"));  // the classic trap
    CHECK_EQ(idioms::which(std::move(b)), std::string_view("const lvalue"));  // const&& -> const&
}

TEST(relay_preserves_category) {
    std::string a = "a";
    CHECK_EQ(idioms::relay(a), std::string_view("lvalue"));
    CHECK_EQ(idioms::relay(std::as_const(a)), std::string_view("const lvalue"));
    CHECK_EQ(idioms::relay(std::move(a)), std::string_view("rvalue"));
    CHECK_EQ(idioms::relay(std::string("t")), std::string_view("rvalue"));
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
