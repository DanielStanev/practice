#include "matching_engine.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

#include "testing.hpp"

using hft::match::MatchingEngine;
using hft::match::Side;
using hft::match::Status;
using hft::match::TimeInForce;
using hft::match::Trade;

namespace {

// Brute-force reference engine: a flat list of resting orders with
// arrival timestamps. Slow and obviously correct.
struct ReferenceEngine {
    struct O {
        std::uint64_t id;
        Side side;
        std::int64_t price;
        std::uint32_t qty;
        std::uint64_t time;
    };
    std::vector<O> book;
    std::vector<Trade> trades;
    std::uint64_t clock = 0;

    O* find(std::uint64_t id) {
        for (auto& o : book)
            if (o.id == id) return &o;
        return nullptr;
    }

    std::uint32_t submit(std::uint64_t id, Side side, std::int64_t price, std::uint32_t qty, TimeInForce tif) {
        if (qty == 0 || find(id)) return 0;
        std::uint32_t left = qty;
        while (left > 0) {
            O* best = nullptr;
            for (auto& o : book) {
                if (o.side == side) continue;
                bool crosses = side == Side::Buy ? o.price <= price : o.price >= price;
                if (!crosses) continue;
                if (!best) {
                    best = &o;
                    continue;
                }
                bool better = side == Side::Buy ? o.price < best->price : o.price > best->price;
                if (better || (o.price == best->price && o.time < best->time)) best = &o;
            }
            if (!best) break;
            std::uint32_t f = std::min(left, best->qty);
            trades.push_back(Trade{id, best->id, best->price, f});
            left -= f;
            best->qty -= f;
            if (best->qty == 0) book.erase(book.begin() + (best - book.data()));
        }
        if (left > 0 && tif == TimeInForce::GTC) book.push_back(O{id, side, price, left, clock++});
        return qty - left;
    }
    bool cancel(std::uint64_t id) {
        for (std::size_t i = 0; i < book.size(); ++i)
            if (book[i].id == id) {
                book.erase(book.begin() + i);
                return true;
            }
        return false;
    }
    bool modify(std::uint64_t id, std::int64_t price, std::uint32_t qty) {
        O* o = find(id);
        if (!o || qty == 0) return false;
        if (price == o->price && qty <= o->qty) {
            o->qty = qty;
            return true;
        }
        Side s = o->side;
        cancel(id);
        submit(id, s, price, qty, TimeInForce::GTC);
        return true;
    }
    std::optional<std::int64_t> best(Side s) const {
        std::optional<std::int64_t> b;
        for (auto& o : book)
            if (o.side == s && (!b || (s == Side::Buy ? o.price > *b : o.price < *b))) b = o.price;
        return b;
    }
};

}  // namespace

TEST(non_crossing_orders_rest) {
    MatchingEngine e;
    auto r1 = e.submit(1, Side::Buy, 100, 10);
    auto r2 = e.submit(2, Side::Sell, 101, 5);
    CHECK(r1.status == Status::Accepted);
    CHECK_EQ(r1.filled, 0u);
    CHECK_EQ(r1.resting, 10u);
    CHECK_EQ(r2.resting, 5u);
    CHECK_EQ(*e.best_bid(), 100);
    CHECK_EQ(*e.best_ask(), 101);
    CHECK(e.trades().empty());
    CHECK_EQ(e.resting_orders(), 2u);
}

TEST(rejects_invalid_orders) {
    MatchingEngine e;
    CHECK(e.submit(1, Side::Buy, 100, 0).status == Status::RejectedInvalidQty);
    e.submit(1, Side::Buy, 100, 10);
    CHECK(e.submit(1, Side::Sell, 200, 10).status == Status::RejectedDuplicateId);
    CHECK_EQ(e.resting_orders(), 1u);
}

TEST(trade_at_maker_price) {
    MatchingEngine e;
    e.submit(1, Side::Sell, 100, 10);
    auto r = e.submit(2, Side::Buy, 105, 10);  // willing to pay 105, gets 100
    CHECK_EQ(r.filled, 10u);
    CHECK_EQ(r.resting, 0u);
    CHECK_EQ(e.trades().size(), 1u);
    CHECK(e.trades()[0] == (Trade{2, 1, 100, 10}));
    CHECK_FALSE(e.best_ask().has_value());
    CHECK_FALSE(e.best_bid().has_value());
    CHECK_EQ(e.resting_orders(), 0u);
}

TEST(walks_multiple_levels_and_rests_remainder) {
    MatchingEngine e;
    e.submit(1, Side::Sell, 101, 5);
    e.submit(2, Side::Sell, 102, 5);
    e.submit(3, Side::Sell, 103, 5);
    auto r = e.submit(4, Side::Buy, 102, 20);
    CHECK_EQ(r.filled, 10u);
    CHECK_EQ(r.resting, 10u);
    CHECK_EQ(e.trades().size(), 2u);
    CHECK(e.trades()[0] == (Trade{4, 1, 101, 5}));
    CHECK(e.trades()[1] == (Trade{4, 2, 102, 5}));
    CHECK_EQ(*e.best_bid(), 102);  // remainder rests at its limit
    CHECK_EQ(*e.best_ask(), 103);
    CHECK_EQ(*e.resting_qty(4), 10u);
}

TEST(partial_fill_of_maker) {
    MatchingEngine e;
    e.submit(1, Side::Buy, 50, 100);
    auto r = e.submit(2, Side::Sell, 50, 30);
    CHECK_EQ(r.filled, 30u);
    CHECK_EQ(*e.resting_qty(1), 70u);
    CHECK_EQ(e.volume_at(Side::Buy, 50), 70u);
}

TEST(time_priority_within_level) {
    MatchingEngine e;
    e.submit(1, Side::Sell, 100, 5);
    e.submit(2, Side::Sell, 100, 5);
    e.submit(3, Side::Sell, 100, 5);
    e.submit(9, Side::Buy, 100, 7);
    CHECK_EQ(e.trades().size(), 2u);
    CHECK_EQ(e.trades()[0].maker_id, 1u);
    CHECK_EQ(e.trades()[1].maker_id, 2u);
    CHECK_EQ(e.trades()[1].qty, 2u);
    CHECK_FALSE(e.resting_qty(1).has_value());
    CHECK_EQ(*e.resting_qty(2), 3u);
    CHECK_EQ(*e.resting_qty(3), 5u);
}

TEST(price_priority_beats_time) {
    MatchingEngine e;
    e.submit(1, Side::Buy, 99, 5);
    e.submit(2, Side::Buy, 100, 5);  // later but better price
    e.submit(3, Side::Sell, 99, 5);
    CHECK_EQ(e.trades()[0].maker_id, 2u);
    CHECK_EQ(e.trades()[0].price, 100);
}

TEST(ioc_never_rests) {
    MatchingEngine e;
    e.submit(1, Side::Sell, 100, 5);
    auto r = e.submit(2, Side::Buy, 100, 8, TimeInForce::IOC);
    CHECK_EQ(r.filled, 5u);
    CHECK_EQ(r.resting, 0u);
    CHECK_FALSE(e.best_bid().has_value());
    auto r2 = e.submit(3, Side::Buy, 90, 8, TimeInForce::IOC);
    CHECK(r2.status == Status::Accepted);
    CHECK_EQ(r2.filled, 0u);
    CHECK_EQ(e.resting_orders(), 0u);
}

TEST(market_orders) {
    MatchingEngine e;
    e.submit(1, Side::Sell, 100, 5);
    e.submit(2, Side::Sell, 1000000, 5);
    auto r = e.submit_market(3, Side::Buy, 7);
    CHECK_EQ(r.filled, 7u);
    CHECK_EQ(e.trades().back().price, 1000000);
    auto r2 = e.submit_market(4, Side::Buy, 100);
    CHECK_EQ(r2.filled, 3u);
    CHECK_EQ(r2.resting, 0u);
    CHECK_EQ(e.resting_orders(), 0u);
}

TEST(cancel) {
    MatchingEngine e;
    e.submit(1, Side::Buy, 100, 5);
    e.submit(2, Side::Buy, 100, 5);
    CHECK(e.cancel(1));
    CHECK_FALSE(e.cancel(1));
    CHECK_FALSE(e.cancel(77));
    CHECK_EQ(e.volume_at(Side::Buy, 100), 5u);
    CHECK(e.cancel(2));
    CHECK_FALSE(e.best_bid().has_value());
}

TEST(modify_down_keeps_priority) {
    MatchingEngine e;
    e.submit(1, Side::Sell, 100, 10);
    e.submit(2, Side::Sell, 100, 10);
    CHECK(e.modify(1, 100, 4));
    e.submit(3, Side::Buy, 100, 4);
    CHECK_EQ(e.trades().back().maker_id, 1u);
}

TEST(modify_up_or_price_loses_priority) {
    MatchingEngine e;
    e.submit(1, Side::Sell, 100, 10);
    e.submit(2, Side::Sell, 100, 10);
    CHECK(e.modify(1, 100, 11));  // size up: back of the queue
    e.submit(3, Side::Buy, 100, 1);
    CHECK_EQ(e.trades().back().maker_id, 2u);
    CHECK(e.modify(2, 101, 9));
    CHECK(e.modify(2, 100, 9));  // moved away and back: still behind 1
    e.clear_trades();
    e.submit(4, Side::Buy, 100, 1);
    CHECK_EQ(e.trades().back().maker_id, 1u);
    CHECK_FALSE(e.modify(99, 1, 1));
    CHECK_FALSE(e.modify(1, 100, 0));
}

TEST(modify_can_cross) {
    MatchingEngine e;
    e.submit(1, Side::Buy, 100, 10);
    e.submit(2, Side::Sell, 105, 4);
    CHECK(e.modify(1, 105, 10));  // now aggressive: trades 4, rests 6 at 105
    CHECK(e.trades().back() == (Trade{1, 2, 105, 4}));
    CHECK_EQ(*e.resting_qty(1), 6u);
    CHECK_EQ(*e.best_bid(), 105);
}

TEST(clear_trades) {
    MatchingEngine e;
    e.submit(1, Side::Sell, 1, 1);
    e.submit(2, Side::Buy, 1, 1);
    CHECK_EQ(e.trades().size(), 1u);
    e.clear_trades();
    CHECK(e.trades().empty());
}

TEST(random_against_reference) {
    std::mt19937_64 rng(2024);
    MatchingEngine e;
    ReferenceEngine ref;
    std::uint64_t next_id = 1;
    for (int step = 0; step < 30000; ++step) {
        int op = int(rng() % 10);
        if (op < 6) {
            Side s = rng() % 2 ? Side::Buy : Side::Sell;
            std::int64_t px = 95 + std::int64_t(rng() % 11);
            std::uint32_t q = 1 + std::uint32_t(rng() % 20);
            TimeInForce tif = rng() % 5 == 0 ? TimeInForce::IOC : TimeInForce::GTC;
            auto r = e.submit(next_id, s, px, q, tif);
            std::uint32_t ref_filled = ref.submit(next_id, s, px, q, tif);
            CHECK_EQ(r.filled, ref_filled);
            ++next_id;
        } else if (op < 8) {
            std::uint64_t id = 1 + rng() % next_id;
            CHECK_EQ(e.cancel(id), ref.cancel(id));
        } else {
            std::uint64_t id = 1 + rng() % next_id;
            std::int64_t px = 95 + std::int64_t(rng() % 11);
            std::uint32_t q = std::uint32_t(rng() % 20);
            CHECK_EQ(e.modify(id, px, q), ref.modify(id, px, q));
        }
        // The book is never crossed.
        if (e.best_bid() && e.best_ask()) CHECK(*e.best_bid() < *e.best_ask());
        if (step % 100 == 0) {
            CHECK(e.trades().size() == ref.trades.size());
            CHECK(e.trades() == ref.trades);
            CHECK(e.best_bid() == ref.best(Side::Buy));
            CHECK(e.best_ask() == ref.best(Side::Sell));
            CHECK_EQ(e.resting_orders(), ref.book.size());
            for (auto& o : ref.book) CHECK_EQ(*e.resting_qty(o.id), o.qty);
        }
    }
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
