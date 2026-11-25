#include "order_store.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <random>

#include "testing.hpp"

using hft::AosOrderStore;
using hft::Side;
using hft::SoaOrderStore;

// Every test runs against both layouts: they must behave identically.
#define BOTH_LAYOUTS(name)                     \
    template <typename Store>                  \
    void name##_impl();                        \
    TEST(name##_aos) { name##_impl<AosOrderStore>(); } \
    TEST(name##_soa) { name##_impl<SoaOrderStore>(); } \
    template <typename Store>                  \
    void name##_impl()

BOTH_LAYOUTS(add_and_get) {
    Store s(16);
    CHECK_EQ(s.size(), 0u);
    CHECK(s.add(1, Side::Buy, 1000, 10, 100));
    CHECK(s.add(2, Side::Sell, 1010, 5, 101));
    CHECK_FALSE(s.add(1, Side::Sell, 1, 1, 1));  // duplicate id
    CHECK_FALSE(s.add(3, Side::Buy, 1000, 0, 1));  // zero qty
    CHECK_EQ(s.size(), 2u);
    auto o = s.get(2);
    CHECK(o.has_value());
    CHECK_EQ(o->id, 2u);
    CHECK(o->side == Side::Sell);
    CHECK_EQ(o->price, 1010);
    CHECK_EQ(o->qty, 5u);
    CHECK_EQ(o->timestamp, 101u);
    CHECK_FALSE(s.get(99).has_value());
}

BOTH_LAYOUTS(remove_swaps_last_into_hole) {
    Store s;
    for (std::uint64_t i = 1; i <= 5; ++i) s.add(i, Side::Buy, std::int64_t(i * 10), 1, i);
    CHECK(s.remove(2));
    CHECK_FALSE(s.remove(2));
    CHECK_EQ(s.size(), 4u);
    // The order moved into the hole must still be found by id.
    for (std::uint64_t i : {1u, 3u, 4u, 5u}) {
        auto o = s.get(i);
        CHECK(o.has_value());
        CHECK_EQ(o->price, std::int64_t(i * 10));
    }
    CHECK(s.remove(5));  // remove the last element
    CHECK(s.remove(1));
    CHECK_EQ(s.size(), 2u);
    CHECK_EQ(s.get(4)->price, 40);
}

BOTH_LAYOUTS(modify_qty) {
    Store s;
    s.add(7, Side::Sell, 500, 10, 0);
    CHECK(s.modify_qty(7, 3));
    CHECK_EQ(s.get(7)->qty, 3u);
    CHECK_FALSE(s.modify_qty(8, 3));
    CHECK(s.modify_qty(7, 0));  // zero removes
    CHECK_FALSE(s.get(7).has_value());
    CHECK_EQ(s.size(), 0u);
}

BOTH_LAYOUTS(aggregate_queries) {
    Store s;
    s.add(1, Side::Buy, 100, 10, 5);
    s.add(2, Side::Buy, 101, 20, 6);
    s.add(3, Side::Buy, 99, 30, 7);
    s.add(4, Side::Sell, 103, 1, 8);
    s.add(5, Side::Sell, 102, 2, 9);
    s.add(6, Side::Sell, 105, 4, 10);
    CHECK_EQ(s.total_qty(Side::Buy), 60u);
    CHECK_EQ(s.total_qty(Side::Sell), 7u);
    // Buy side: "better" means higher price.
    CHECK_EQ(s.qty_at_or_better(Side::Buy, 100), 30u);
    CHECK_EQ(s.qty_at_or_better(Side::Buy, 102), 0u);
    // Sell side: "better" means lower price.
    CHECK_EQ(s.qty_at_or_better(Side::Sell, 103), 3u);
    CHECK_EQ(s.qty_at_or_better(Side::Sell, 101), 0u);
    CHECK_EQ(*s.best_price(Side::Buy), 101);
    CHECK_EQ(*s.best_price(Side::Sell), 102);
    CHECK_EQ(s.count_older_than(8), 3u);
    Store empty;
    CHECK_FALSE(empty.best_price(Side::Buy).has_value());
    CHECK_EQ(empty.total_qty(Side::Sell), 0u);
}

BOTH_LAYOUTS(random_against_reference) {
    struct Ref {
        Side side;
        std::int64_t price;
        std::uint32_t qty;
        std::uint64_t ts;
    };
    Store s(1024);
    std::map<std::uint64_t, Ref> ref;
    std::mt19937_64 rng(17);
    for (int step = 0; step < 50000; ++step) {
        std::uint64_t id = rng() % 2000;
        switch (rng() % 4) {
            case 0:
            case 1: {
                Side side = rng() % 2 ? Side::Buy : Side::Sell;
                std::int64_t px = 1000 + std::int64_t(rng() % 50);
                std::uint32_t q = 1 + std::uint32_t(rng() % 100);
                bool added = s.add(id, side, px, q, std::uint64_t(step));
                bool ref_added = ref.emplace(id, Ref{side, px, q, std::uint64_t(step)}).second;
                CHECK_EQ(added, ref_added);
                break;
            }
            case 2:
                CHECK_EQ(s.remove(id), ref.erase(id) == 1);
                break;
            case 3: {
                std::uint32_t q = std::uint32_t(rng() % 50);
                bool exists = ref.count(id) == 1;
                CHECK_EQ(s.modify_qty(id, q), exists);
                if (exists) {
                    if (q == 0) ref.erase(id);
                    else ref[id].qty = q;
                }
                break;
            }
        }
        if (step % 1000 == 0) {
            CHECK_EQ(s.size(), ref.size());
            std::uint64_t buy = 0, sell = 0, buy_better = 0;
            std::optional<std::int64_t> best_sell;
            std::size_t old = 0;
            for (auto& [k, r] : ref) {
                (r.side == Side::Buy ? buy : sell) += r.qty;
                if (r.side == Side::Buy && r.price >= 1025) buy_better += r.qty;
                if (r.side == Side::Sell && (!best_sell || r.price < *best_sell)) best_sell = r.price;
                old += r.ts < std::uint64_t(step / 2);
            }
            CHECK_EQ(s.total_qty(Side::Buy), buy);
            CHECK_EQ(s.total_qty(Side::Sell), sell);
            CHECK_EQ(s.qty_at_or_better(Side::Buy, 1025), buy_better);
            CHECK(s.best_price(Side::Sell) == best_sell);
            CHECK_EQ(s.count_older_than(std::uint64_t(step / 2)), old);
            for (auto& [k, r] : ref) {
                auto o = s.get(k);
                CHECK(o.has_value());
                CHECK_EQ(o->qty, r.qty);
                CHECK_EQ(o->price, r.price);
            }
        }
    }
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
