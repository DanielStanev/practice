#include "clone_ptr.hpp"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
struct Stock : Instrument {
    static inline int alive = 0;
    double px;
    long qty;
    Stock(double p, long q) : px(p), qty(q) { ++alive; }
    Stock(const Stock& o) : Instrument(o), px(o.px), qty(o.qty) { ++alive; }
    ~Stock() override { --alive; }
    double value() const override { return px * static_cast<double>(qty); }
};
struct Cash : Instrument {
    double amount;
    explicit Cash(double a) : amount(a) {}
    double value() const override { return amount; }
};
}  // namespace

TEST(make_and_access) {
    auto p = ClonePtr<Instrument>::make<Stock>(10.0, 3L);
    CHECK(static_cast<bool>(p));
    CHECK_EQ(p->value(), 30.0);
    CHECK_EQ((*p).value(), 30.0);
    ClonePtr<Instrument> empty;
    CHECK_FALSE(static_cast<bool>(empty));
}

TEST(copy_is_deep_through_dynamic_type) {
    Stock::alive = 0;
    {
        auto a = ClonePtr<Instrument>::make<Stock>(10.0, 3L);
        ClonePtr<Instrument> b = a;
        CHECK_EQ(Stock::alive, 2);
        CHECK(a.get() != b.get());
        static_cast<Stock*>(b.get())->qty = 5;
        CHECK_EQ(a->value(), 30.0);
        CHECK_EQ(b->value(), 50.0);
        ClonePtr<Instrument> empty;
        ClonePtr<Instrument> c = empty;  // copying empty is fine
        CHECK_FALSE(static_cast<bool>(c));
        c = a;
        CHECK_EQ(Stock::alive, 3);
        c = ClonePtr<Instrument>::make<Cash>(7.0);
        CHECK_EQ(Stock::alive, 2);
        CHECK_EQ(c->value(), 7.0);
    }
    CHECK_EQ(Stock::alive, 0);
}

TEST(move_steals) {
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<ClonePtr<Instrument>>);
    CHECK_TRAIT(std::is_nothrow_move_assignable_v<ClonePtr<Instrument>>);
    auto a = ClonePtr<Instrument>::make<Cash>(1.0);
    Instrument* raw = a.get();
    ClonePtr<Instrument> b(std::move(a));
    CHECK(b.get() == raw);
    CHECK(a.get() == nullptr);
    ClonePtr<Instrument> c;
    c = std::move(b);
    CHECK(c.get() == raw);
    ClonePtr<Instrument> d = a;  // copying a moved-from (empty) pointer
    CHECK_FALSE(static_cast<bool>(d));
}

TEST(portfolio_rule_of_zero) {
    CHECK_TRAIT(std::is_copy_constructible_v<Portfolio>);
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<Portfolio>);
    Portfolio p;
    p.name = "book";
    p.add<Stock>(100.0, 10L);
    p.add<Cash>(50.0);
    CHECK_EQ(p.value(), 1050.0);
    Portfolio q = p;
    static_cast<Stock*>(q.legs[0].get())->qty = 20;
    CHECK_EQ(p.value(), 1050.0);
    CHECK_EQ(q.value(), 2050.0);
    Portfolio r = std::move(q);
    CHECK_EQ(r.value(), 2050.0);
    CHECK(q.legs.empty());
}

TEST(legacy_book_moves) {
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<LegacyBook>);
    LegacyBook a;
    a.orders.assign(100, std::string(50, 'x'));
    const std::string* data = a.orders.data();
    LegacyBook b(std::move(a));
    CHECK(b.orders.data() == data);  // buffer stolen, not copied
    LegacyBook c;
    c = std::move(b);
    CHECK(c.orders.data() == data);
    LegacyBook d = c;  // still copyable
    CHECK_EQ(d.orders.size(), std::size_t{100});
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
