#include "crtp_mixins.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace crtp_test {
struct Price : LessThanComparable<Price> {
    long ticks;
    explicit Price(long t) : ticks(t) {}
    friend bool operator<(const Price& a, const Price& b) { return a.ticks < b.ticks; }
};

struct Order : InstanceCounter<Order> {
    int id = 0;
};
struct Fill : InstanceCounter<Fill> {
    int id = 0;
};

struct Instrument {
    virtual ~Instrument() = default;
    virtual std::unique_ptr<Instrument> clone() const = 0;
    virtual double value() const = 0;
};
struct Stock : Cloneable<Instrument, Stock> {
    double px = 0;
    explicit Stock(double p) : px(p) {}
    double value() const override { return px; }
};
struct Bond : Cloneable<Instrument, Bond> {
    double face = 0;
    explicit Bond(double f) : face(f) {}
    double value() const override { return face * 0.98; }
};

struct Quote : Printable<Quote> {
    std::string sym;
    double bid, ask;
    Quote(std::string s, double b, double a) : sym(std::move(s)), bid(b), ask(a) {}
    static std::string_view name() { return "Quote"; }
    void fields(std::ostream& os) const { os << sym << ',' << bid << ',' << ask; }
};

// Deriving from the wrong instantiation must not compile.
struct Good : LessThanComparable<Good> {
    friend bool operator<(const Good&, const Good&) { return false; }
};
struct Bad : LessThanComparable<Good> {};
}  // namespace crtp_test
using namespace crtp_test;

TEST(comparisons_from_less_than) {
    Price a(100), b(101), c(100);
    CHECK(a < b);
    CHECK(b > a);
    CHECK(a <= c);
    CHECK(a >= c);
    CHECK(a == c);
    CHECK(a != b);
    CHECK_FALSE(a > b);
    CHECK_FALSE(b <= a);
    std::vector<Price> v{Price(3), Price(1), Price(2)};
    std::sort(v.begin(), v.end(), [](const Price& x, const Price& y) { return x > y; });
    CHECK_EQ(v.front().ticks, 3L);
}

TEST(wrong_base_does_not_compile) {
    CHECK_TRAIT(std::is_default_constructible_v<Good>);
    CHECK_TRAIT(!std::is_default_constructible_v<Bad>);
}

TEST(counters_are_per_type) {
    long o0 = Order::created(), f0 = Fill::created();
    {
        Order a, b;
        Order c = a;
        Order d = std::move(b);
        Fill f;
        CHECK_EQ(Order::created() - o0, 4L);
        CHECK_EQ(Order::alive(), 4L);
        CHECK_EQ(Fill::created() - f0, 1L);
        CHECK_EQ(Fill::alive(), 1L);
        c = d;  // assignment creates nothing
        CHECK_EQ(Order::created() - o0, 4L);
    }
    CHECK_EQ(Order::alive(), 0L);
    CHECK_EQ(Fill::alive(), 0L);
}

TEST(clone_through_base) {
    std::vector<std::unique_ptr<Instrument>> book;
    book.push_back(std::make_unique<Stock>(10.0));
    book.push_back(std::make_unique<Bond>(100.0));
    std::vector<std::unique_ptr<Instrument>> copy;
    for (auto& p : book) copy.push_back(p->clone());
    CHECK(dynamic_cast<Stock*>(copy[0].get()) != nullptr);
    CHECK(dynamic_cast<Bond*>(copy[1].get()) != nullptr);
    CHECK(copy[0].get() != book[0].get());
    CHECK_EQ(copy[1]->value(), 98.0);
}

TEST(printable) {
    Quote q("AAPL", 189.5, 189.75);
    CHECK_EQ(q.to_string(), std::string("Quote{AAPL,189.5,189.75}"));
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
