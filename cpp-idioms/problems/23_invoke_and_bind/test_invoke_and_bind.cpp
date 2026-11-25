#include "invoke_and_bind.hpp"

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
struct Order {
    int qty = 0;
    int price = 0;
    int notional() const { return qty * price; }
    int scaled(int k) const { return qty * k; }
    void bump(int by) { qty += by; }
    std::string tag() && { return "rvalue"; }
    std::string tag() & { return "lvalue"; }
};
struct Derived : Order {};

int add3(int a, int b, int c) { return a + b + c; }
}  // namespace

TEST(plain_callables) {
    CHECK_EQ(my_invoke(add3, 1, 2, 3), 6);
    CHECK_EQ(my_invoke([](int x) { return x * 2; }, 21), 42);
    std::function<int(int)> f = [](int x) { return -x; };
    CHECK_EQ(my_invoke(f, 5), -5);
}

TEST(member_functions_all_object_forms) {
    Order o{3, 7};
    Order* raw = &o;
    auto sp = std::make_shared<Order>(Order{2, 5});
    auto up = std::make_unique<Order>(Order{4, 1});
    CHECK_EQ(my_invoke(&Order::notional, o), 21);
    CHECK_EQ(my_invoke(&Order::notional, raw), 21);
    CHECK_EQ(my_invoke(&Order::notional, sp), 10);
    CHECK_EQ(my_invoke(&Order::notional, up), 4);
    CHECK_EQ(my_invoke(&Order::notional, std::ref(o)), 21);
    CHECK_EQ(my_invoke(&Order::scaled, o, 10), 30);
    my_invoke(&Order::bump, o, 5);
    CHECK_EQ(o.qty, 8);
    my_invoke(&Order::bump, std::ref(o), 1);
    CHECK_EQ(o.qty, 9);
    Derived d;
    d.qty = 1;
    d.price = 1;
    CHECK_EQ(my_invoke(&Order::notional, d), 1);  // base-class member on a derived object
}

TEST(member_function_ref_qualifiers) {
    // &Order::tag is overloaded, so pick each overload by casting to its exact type.
    using LTag = std::string (Order::*)() &;
    using RTag = std::string (Order::*)() &&;
    LTag ltag = &Order::tag;
    RTag rtag = &Order::tag;
    Order o;
    CHECK_EQ(my_invoke(ltag, o), std::string("lvalue"));
    CHECK_EQ(my_invoke(rtag, std::move(o)), std::string("rvalue"));  // needs the object as an rvalue
    CHECK_EQ(my_invoke(rtag, Order{}), std::string("rvalue"));
}

TEST(data_members_keep_category) {
    Order o{3, 7};
    const Order co{1, 1};
    CHECK_TRAIT(std::is_same_v<decltype(my_invoke(&Order::qty, o)), int&>);
    CHECK_TRAIT(std::is_same_v<decltype(my_invoke(&Order::qty, co)), const int&>);
    CHECK_TRAIT(std::is_same_v<decltype(my_invoke(&Order::qty, std::move(o))), int&&>);
    CHECK_TRAIT(std::is_same_v<decltype(my_invoke(&Order::qty, &o)), int&>);
    my_invoke(&Order::qty, o) = 11;
    CHECK_EQ(o.qty, 11);
    CHECK_EQ(my_invoke(&Order::price, std::make_unique<Order>(Order{0, 9})), 9);
}

TEST(bind_front_values) {
    auto add_10_20 = my_bind_front(add3, 10, 20);
    CHECK_EQ(add_10_20(3), 33);
    Order o{2, 50};
    auto scale_o = my_bind_front(&Order::scaled, std::ref(o));
    o.qty = 4;  // bound by reference
    CHECK_EQ(scale_o(3), 12);
    auto copy_o = my_bind_front(&Order::notional, o);  // bound by value (a copy)
    o.qty = 100;
    CHECK_EQ(copy_o(), 200);
    auto owns = my_bind_front([](const std::unique_ptr<int>& p, int k) { return *p * k; }, std::make_unique<int>(6));
    CHECK_EQ(owns(7), 42);
}

TEST(sum_by_projections) {
    std::vector<Order> v{{1, 10}, {2, 20}, {3, 30}};
    CHECK_EQ(sum_by(v, &Order::qty), 6L);
    CHECK_EQ(sum_by(v, &Order::notional), 140L);
    CHECK_EQ(sum_by(v, [](const Order& o) { return o.price; }), 60L);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
