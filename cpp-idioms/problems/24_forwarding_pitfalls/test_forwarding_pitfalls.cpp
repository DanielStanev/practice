#include "forwarding_pitfalls.hpp"

#include <cstdint>
#include <ranges>
#include <string>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
struct DerivedNamed : Named {
    using Named::Named;
};

struct Sink {
    std::vector<std::string>* log;
    void operator()(const std::string& s) const { log->push_back("lvalue:" + s); }
    void operator()(std::string&& s) const {
        log->push_back("rvalue:" + s);
        std::string stolen = std::move(s);  // takes it
    }
};
}  // namespace

TEST(named_traits) {
    CHECK_TRAIT(std::is_copy_constructible_v<Named>);
    CHECK_TRAIT(std::is_constructible_v<Named, const char*>);
    CHECK_TRAIT(std::is_constructible_v<Named, std::string_view>);
    CHECK_TRAIT(!std::is_constructible_v<Named, int>);
}

TEST(named_copy_uses_copy_constructor) {
    Named a("alpha");
    Named::converting_calls = 0;
    Named b(a);  // non-const lvalue: the unconstrained template is a better match than Named(const Named&)
    CHECK_EQ(b.name, std::string("alpha"));
    CHECK_EQ(Named::converting_calls, 0);
    DerivedNamed d("derived");
    Named::converting_calls = 0;
    Named e(d);
    Named f(std::move(d));
    CHECK_EQ(e.name, std::string("derived"));
    CHECK_EQ(f.name, std::string("derived"));
    CHECK_EQ(Named::converting_calls, 0);
    std::string s = "moved in";
    Named g(std::move(s));
    CHECK_EQ(g.name, std::string("moved in"));
    CHECK_EQ(Named::converting_calls, 1);
}

TEST(logger_overloads) {
    Logger l;
    short s = 7;
    std::size_t z = 9;
    std::int8_t tiny = 3;
    std::string text = "hello";
    l.log(42);
    l.log(s);
    l.log(z);
    l.log(tiny);
    l.log(text);
    l.log("literal");
    l.log(std::string_view("sv"));
    std::vector<std::string> expect{"id:42", "id:7", "id:9", "id:3", "text:hello", "text:literal", "text:sv"};
    CHECK(l.entries() == expect);
}

TEST(broadcast_forwards_only_last) {
    std::vector<std::string> log;
    Sink s{&log};
    std::size_t n = broadcast(std::string("msg"), s, s, s);
    CHECK_EQ(n, std::size_t{3});
    CHECK(log == (std::vector<std::string>{"lvalue:msg", "lvalue:msg", "rvalue:msg"}));
    log.clear();
    std::string keep = "mine";
    broadcast(keep, s, s);  // lvalue argument: never moved from
    CHECK(log == (std::vector<std::string>{"lvalue:mine", "lvalue:mine"}));
    CHECK_EQ(keep, std::string("mine"));
    CHECK_EQ(broadcast(1), std::size_t{0});
}

TEST(assign_all_any_reference_type) {
    std::vector<int> ints(4, 0);
    assign_all(ints, 5);
    CHECK(ints == (std::vector<int>{5, 5, 5, 5}));
    std::vector<bool> bits(10, false);
    assign_all(bits, true);
    CHECK(bits == std::vector<bool>(10, true));
    int a = 0, b = 0;
    std::vector<int*> ptrs{&a, &b};
    assign_all(ptrs | std::views::transform([](int* p) -> int& { return *p; }), 9);
    CHECK_EQ(a + b, 18);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
