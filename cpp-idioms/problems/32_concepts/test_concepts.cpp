#include "concepts.hpp"

#include <compare>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "testing.hpp"

using namespace idioms;

// A named namespace (not anonymous) so unused operators do not trigger warnings.
namespace concepts_test {
struct Ticks {
    long v = 0;
    auto operator<=>(const Ticks&) const = default;
    friend Ticks operator+(Ticks a, Ticks b) { return {a.v + b.v}; }
    friend Ticks operator-(Ticks a, Ticks b) { return {a.v - b.v}; }
};
struct NoMinus {
    long v = 0;
    auto operator<=>(const NoMinus&) const = default;
    friend NoMinus operator+(NoMinus a, NoMinus b) { return {a.v + b.v}; }
};
struct Unordered {
    long v = 0;
    bool operator==(const Unordered&) const = default;
    friend Unordered operator+(Unordered a, Unordered b) { return {a.v + b.v}; }
    friend Unordered operator-(Unordered a, Unordered b) { return {a.v - b.v}; }
};

template <class P>
struct MapBook {
    using price_type = P;
    std::map<P, long> bids, asks;
    void add(Side s, P p, long q) { (s == Side::buy ? bids : asks)[p] += q; }
    std::optional<P> best_bid() const { return bids.empty() ? std::nullopt : std::optional<P>(bids.rbegin()->first); }
    std::optional<P> best_ask() const { return asks.empty() ? std::nullopt : std::optional<P>(asks.begin()->first); }
};
struct NoAdd {
    using price_type = long;
    std::optional<long> best_bid() const { return {}; }
    std::optional<long> best_ask() const { return {}; }
};
struct MutableBestBid {  // best_bid() is not const
    using price_type = long;
    void add(Side, long, long) {}
    std::optional<long> best_bid() { return {}; }
    std::optional<long> best_ask() const { return {}; }
};

template <class P>
concept LevelValid = requires { typename Level<P>; };

template <class B>
void spread_impl() {
    if constexpr (Book<B>) {
        B b;
        CHECK_FALSE(spread(b).has_value());
        b.add(Side::buy, typename B::price_type{100}, 5);
        CHECK_FALSE(spread(b).has_value());
        b.add(Side::sell, typename B::price_type{103}, 5);
        b.add(Side::sell, typename B::price_type{102}, 5);
        b.add(Side::buy, typename B::price_type{99}, 5);
        auto s = spread(b);
        CHECK(s.has_value());
        CHECK(*s == typename B::price_type{2});
    } else {
        ::testing::fail(__FILE__, __LINE__, "Book concept not satisfied yet");
    }
}
}  // namespace concepts_test
using namespace concepts_test;

TEST(price_concept) {
    CHECK_TRAIT(Price<int>);
    CHECK_TRAIT(Price<double>);
    CHECK_TRAIT(Price<Ticks>);
    CHECK_TRAIT(!Price<NoMinus>);
    CHECK_TRAIT(!Price<Unordered>);
    CHECK_TRAIT(!Price<std::string>);  // + yes, - no
    CHECK_TRAIT(!Price<int&>);        // references are not regular values
}

TEST(book_concept) {
    CHECK_TRAIT(Book<MapBook<long>>);
    CHECK_TRAIT(Book<MapBook<Ticks>>);
    CHECK_TRAIT(!Book<MapBook<NoMinus>>);
    CHECK_TRAIT(!Book<NoAdd>);
    CHECK_TRAIT(!Book<MutableBestBid>);
    CHECK_TRAIT(!Book<int>);
}

TEST(spread_long) { spread_impl<MapBook<long>>(); }

TEST(describe_by_subsumption) {
    CHECK_EQ(describe(5), std::string_view("signed integral"));
    CHECK_EQ(describe(5u), std::string_view("integral"));
    CHECK_EQ(describe(2.5), std::string_view("floating"));
    CHECK_EQ(describe(Ticks{3}), std::string_view("price"));
    CHECK_EQ(describe(std::string("x")), std::string_view("other"));
    CHECK_EQ(describe(NoMinus{}), std::string_view("other"));
}

TEST(constrained_class_template) {
    CHECK_TRAIT(LevelValid<Ticks>);
    CHECK_TRAIT(LevelValid<double>);
    CHECK_TRAIT(!LevelValid<std::string>);
}

TEST(total_values) {
    std::vector<Ticks> t{{1}, {2}, {3}};
    CHECK_EQ(total(t).v, 6L);
    CHECK_EQ(total(std::vector<double>{0.5, 0.25}), 0.75);
    CHECK_EQ(total(std::vector<int>{}), 0);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
