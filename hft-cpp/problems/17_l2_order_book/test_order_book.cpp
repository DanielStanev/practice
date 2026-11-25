#include "order_book.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <span>
#include <vector>

#include "testing.hpp"

using hft::book::BookBuilder;
using hft::book::Level;
using hft::book::OrderBook;
using hft::book::Side;
namespace itch = hft::itch;

namespace {

// Deliberately naive reference: a list of orders, aggregated on demand.
struct ReferenceBook {
    struct O {
        Side side;
        std::int64_t price;
        std::uint32_t qty;
    };
    std::map<std::uint64_t, O> orders;

    bool add(std::uint64_t ref, Side s, std::int64_t p, std::uint32_t q) {
        if (q == 0) return false;
        return orders.emplace(ref, O{s, p, q}).second;
    }
    bool reduce(std::uint64_t ref, std::uint32_t q) {
        auto it = orders.find(ref);
        if (it == orders.end()) return false;
        if (q >= it->second.qty) orders.erase(it);
        else it->second.qty -= q;
        return true;
    }
    bool remove(std::uint64_t ref) { return orders.erase(ref) == 1; }
    bool replace(std::uint64_t ref, std::uint64_t nref, std::uint32_t q, std::int64_t p) {
        auto it = orders.find(ref);
        if (it == orders.end() || (nref != ref && orders.count(nref))) return false;
        Side s = it->second.side;
        orders.erase(it);
        return add(nref, s, p, q);
    }
    std::vector<Level> levels(Side s) const {
        std::map<std::int64_t, Level> agg;
        for (auto& [r, o] : orders) {
            if (o.side != s) continue;
            auto& l = agg[o.price];
            l.price = o.price;
            l.qty += o.qty;
            l.orders += 1;
        }
        std::vector<Level> v;
        for (auto& [p, l] : agg) v.push_back(l);
        if (s == Side::Buy) std::reverse(v.begin(), v.end());
        return v;
    }
};

std::vector<Level> all_levels(const OrderBook& b, Side s) {
    std::vector<Level> v(b.depth(s) + 5);
    v.resize(b.levels(s, v));
    return v;
}

std::array<char, 8> stock(const char* s) {
    std::array<char, 8> a;
    a.fill(' ');
    for (std::size_t i = 0; s[i] && i < 8; ++i) a[i] = s[i];
    return a;
}

}  // namespace

TEST(empty_book) {
    OrderBook b;
    CHECK_FALSE(b.best_bid().has_value());
    CHECK_FALSE(b.best_ask().has_value());
    CHECK_EQ(b.depth(Side::Buy), 0u);
    CHECK_EQ(b.order_count(), 0u);
}

TEST(add_and_best_prices) {
    OrderBook b;
    CHECK(b.add(1, Side::Buy, 1000, 10));
    CHECK(b.add(2, Side::Buy, 1001, 5));
    CHECK(b.add(3, Side::Sell, 1003, 7));
    CHECK(b.add(4, Side::Sell, 1002, 2));
    CHECK(*b.best_bid() == (Level{1001, 5, 1}));
    CHECK(*b.best_ask() == (Level{1002, 2, 1}));
    CHECK_EQ(b.order_count(), 4u);
    CHECK_FALSE(b.add(1, Side::Sell, 1, 1));  // duplicate ref
    CHECK_FALSE(b.add(9, Side::Sell, 1, 0));  // zero qty
}

TEST(levels_aggregate_orders) {
    OrderBook b;
    b.add(1, Side::Buy, 1000, 10);
    b.add(2, Side::Buy, 1000, 15);
    b.add(3, Side::Buy, 999, 1);
    CHECK(*b.best_bid() == (Level{1000, 25, 2}));
    CHECK_EQ(b.qty_at(Side::Buy, 1000), 25u);
    CHECK_EQ(b.qty_at(Side::Buy, 998), 0u);
    CHECK_EQ(b.qty_at(Side::Sell, 1000), 0u);
    CHECK_EQ(b.depth(Side::Buy), 2u);
}

TEST(levels_best_first) {
    OrderBook b;
    for (int i = 0; i < 5; ++i) {
        b.add(std::uint64_t(i), Side::Buy, 100 + i, 1);
        b.add(std::uint64_t(100 + i), Side::Sell, 200 + i, 1);
    }
    Level out[3];
    CHECK_EQ(b.levels(Side::Buy, out), 3u);
    CHECK_EQ(out[0].price, 104);
    CHECK_EQ(out[2].price, 102);
    CHECK_EQ(b.levels(Side::Sell, out), 3u);
    CHECK_EQ(out[0].price, 200);
    CHECK_EQ(out[2].price, 202);
    Level many[10];
    CHECK_EQ(b.levels(Side::Sell, many), 5u);
}

TEST(execute_and_cancel_reduce) {
    OrderBook b;
    b.add(1, Side::Sell, 500, 100);
    b.add(2, Side::Sell, 500, 50);
    CHECK(b.execute(1, 30));
    CHECK(*b.best_ask() == (Level{500, 120, 2}));
    CHECK_EQ(*b.order_qty(1), 70u);
    CHECK(b.cancel(1, 70));  // fully cancelled: order disappears
    CHECK(*b.best_ask() == (Level{500, 50, 1}));
    CHECK_FALSE(b.order_qty(1).has_value());
    CHECK(b.execute(2, 999));  // more than remaining: order gone
    CHECK_FALSE(b.best_ask().has_value());
    CHECK_EQ(b.depth(Side::Sell), 0u);  // empty level removed
    CHECK_FALSE(b.execute(2, 1));
    CHECK_FALSE(b.cancel(42, 1));
}

TEST(remove_order) {
    OrderBook b;
    b.add(1, Side::Buy, 10, 1);
    b.add(2, Side::Buy, 11, 1);
    CHECK(b.remove(2));
    CHECK_EQ(b.best_bid()->price, 10);
    CHECK_FALSE(b.remove(2));
    CHECK_EQ(b.order_count(), 1u);
}

TEST(replace_moves_order) {
    OrderBook b;
    b.add(1, Side::Buy, 100, 10);
    CHECK(b.replace(1, 2, 25, 105));
    CHECK_FALSE(b.order_qty(1).has_value());
    CHECK_EQ(*b.order_qty(2), 25u);
    CHECK(*b.best_bid() == (Level{105, 25, 1}));
    CHECK_EQ(b.qty_at(Side::Buy, 100), 0u);
    CHECK_FALSE(b.replace(1, 3, 1, 1));  // unknown original
    b.add(3, Side::Sell, 200, 1);
    CHECK_FALSE(b.replace(2, 3, 1, 1));  // new ref already in use
    CHECK_EQ(*b.order_qty(2), 25u);      // and nothing changed
}

TEST(random_against_reference) {
    std::mt19937_64 rng(1234);
    OrderBook b;
    ReferenceBook ref;
    std::uint64_t next_ref = 1;
    std::vector<std::uint64_t> refs;
    for (int step = 0; step < 100000; ++step) {
        int op = int(rng() % 10);
        std::uint64_t r = refs.empty() ? 0 : refs[rng() % refs.size()];
        if (op < 4 || refs.empty()) {
            Side s = rng() % 2 ? Side::Buy : Side::Sell;
            std::int64_t p = s == Side::Buy ? 990 + std::int64_t(rng() % 20) : 1001 + std::int64_t(rng() % 20);
            std::uint32_t q = 1 + std::uint32_t(rng() % 500);
            CHECK_EQ(b.add(next_ref, s, p, q), ref.add(next_ref, s, p, q));
            refs.push_back(next_ref++);
        } else if (op < 6) {
            std::uint32_t q = 1 + std::uint32_t(rng() % 300);
            CHECK_EQ(b.execute(r, q), ref.reduce(r, q));
        } else if (op < 7) {
            std::uint32_t q = 1 + std::uint32_t(rng() % 300);
            CHECK_EQ(b.cancel(r, q), ref.reduce(r, q));
        } else if (op < 9) {
            CHECK_EQ(b.remove(r), ref.remove(r));
        } else {
            std::int64_t p = 990 + std::int64_t(rng() % 30);
            std::uint32_t q = 1 + std::uint32_t(rng() % 500);
            CHECK_EQ(b.replace(r, next_ref, q, p), ref.replace(r, next_ref, q, p));
            refs.push_back(next_ref++);
        }
        if (step % 500 == 0) {
            CHECK_EQ(b.order_count(), ref.orders.size());
            for (Side s : {Side::Buy, Side::Sell}) {
                auto want = ref.levels(s);
                auto got = all_levels(b, s);
                CHECK_EQ(got.size(), want.size());
                for (std::size_t i = 0; i < want.size(); ++i) CHECK(got[i] == want[i]);
            }
        }
    }
}

TEST(builder_applies_itch_messages_for_one_stock) {
    OrderBook b;
    BookBuilder builder(b, "AAPL");
    builder.on_add(itch::AddOrder{1, 10, itch::Side::Buy, 100, stock("AAPL"), 1500000});
    builder.on_add(itch::AddOrder{2, 11, itch::Side::Sell, 200, stock("AAPL"), 1500100});
    builder.on_add(itch::AddOrder{3, 12, itch::Side::Buy, 999, stock("MSFT"), 1600000});  // filtered
    CHECK_EQ(b.order_count(), 2u);
    CHECK_EQ(b.best_bid()->price, 1500000);
    builder.on_execute(itch::OrderExecuted{4, 10, 40, 1});
    CHECK_EQ(b.best_bid()->qty, 60u);
    builder.on_cancel(itch::OrderCancel{5, 11, 50});
    CHECK_EQ(b.best_ask()->qty, 150u);
    builder.on_replace(itch::OrderReplace{6, 11, 13, 75, 1500050});
    CHECK(*b.best_ask() == (Level{1500050, 75, 1}));
    builder.on_delete(itch::OrderDelete{7, 12});  // MSFT order: unknown here, ignored
    builder.on_delete(itch::OrderDelete{8, 10});
    CHECK_FALSE(b.best_bid().has_value());
    CHECK_TRAIT(itch::Handler<BookBuilder>);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
