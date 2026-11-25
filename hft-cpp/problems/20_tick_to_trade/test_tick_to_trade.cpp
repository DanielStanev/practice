#include "tick_to_trade.hpp"

#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <vector>

#include "itch_wire.hpp"
#include "testing.hpp"

using namespace hft::t2t;
using hft::book::OrderBook;

namespace {

constexpr std::uint64_t kSec = 1'000'000'000;

RiskLimits loose_limits() {
    return RiskLimits{/*max_order_qty*/ 1000, /*max_position*/ 100000, /*max_orders_per_sec*/ 1000,
                      /*burst*/ 1000, /*price_collar*/ 1'000'000};
}

std::span<const std::byte> view(const std::vector<std::byte>& v) { return {v.data(), v.size()}; }

}  // namespace

// ------------------------------------------------------------ TokenBucket

TEST(token_bucket_burst_then_refill) {
    TokenBucket b(10, 3);  // 10 per second, bursts of 3
    std::uint64_t t = 5 * kSec;
    CHECK(b.try_consume(t));
    CHECK(b.try_consume(t));
    CHECK(b.try_consume(t));
    CHECK_FALSE(b.try_consume(t));  // burst exhausted
    CHECK_FALSE(b.try_consume(t + kSec / 20));  // 50 ms: half a token
    CHECK(b.try_consume(t + kSec / 10));        // 100 ms: one token
    CHECK_FALSE(b.try_consume(t + kSec / 10));
    CHECK_EQ(b.available(t + 10 * kSec), 3u);  // capped at burst
}

TEST(token_bucket_long_idle_does_not_overflow) {
    TokenBucket b(1'000'000, 5);
    CHECK(b.try_consume(1));
    CHECK_EQ(b.available(~0ull / 2), 5u);
}

// ------------------------------------------------------------ RiskChecker

TEST(risk_accepts_good_order) {
    RiskChecker r(loose_limits());
    CHECK(r.check(OrderRequest{1, Side::Buy, 1000, 10}, 1000, kSec) == RiskResult::Accepted);
}

TEST(risk_max_order_qty) {
    RiskLimits l = loose_limits();
    l.max_order_qty = 100;
    RiskChecker r(l);
    CHECK(r.check(OrderRequest{1, Side::Buy, 1000, 101}, 1000, kSec) == RiskResult::MaxOrderQty);
    CHECK(r.check(OrderRequest{2, Side::Buy, 1000, 0}, 1000, kSec) == RiskResult::MaxOrderQty);
    CHECK(r.check(OrderRequest{3, Side::Buy, 1000, 100}, 1000, kSec) == RiskResult::Accepted);
}

TEST(risk_price_collar) {
    RiskLimits l = loose_limits();
    l.price_collar = 50;
    RiskChecker r(l);
    CHECK(r.check(OrderRequest{1, Side::Buy, 1051, 1}, 1000, kSec) == RiskResult::PriceCollar);
    CHECK(r.check(OrderRequest{2, Side::Sell, 949, 1}, 1000, kSec) == RiskResult::PriceCollar);
    CHECK(r.check(OrderRequest{3, Side::Buy, 1050, 1}, 1000, kSec) == RiskResult::Accepted);
    CHECK(r.check(OrderRequest{4, Side::Buy, 1000, 1}, std::nullopt, kSec) == RiskResult::NoReferencePrice);
}

TEST(risk_position_limit_uses_fills) {
    RiskLimits l = loose_limits();
    l.max_position = 100;
    RiskChecker r(l);
    CHECK(r.check(OrderRequest{1, Side::Buy, 1000, 100}, 1000, kSec) == RiskResult::Accepted);
    r.on_fill(Side::Buy, 100);
    CHECK_EQ(r.position(), 100);
    CHECK(r.check(OrderRequest{2, Side::Buy, 1000, 1}, 1000, kSec) == RiskResult::MaxPosition);
    CHECK(r.check(OrderRequest{3, Side::Sell, 1000, 200}, 1000, kSec) == RiskResult::Accepted);
    r.on_fill(Side::Sell, 200);
    CHECK_EQ(r.position(), -100);
    CHECK(r.check(OrderRequest{4, Side::Sell, 1000, 1}, 1000, kSec) == RiskResult::MaxPosition);
}

TEST(risk_rate_limit) {
    RiskLimits l = loose_limits();
    l.max_orders_per_sec = 2;
    l.burst = 2;
    RiskChecker r(l);
    CHECK(r.check(OrderRequest{1, Side::Buy, 1000, 1}, 1000, kSec) == RiskResult::Accepted);
    CHECK(r.check(OrderRequest{2, Side::Buy, 1000, 1}, 1000, kSec) == RiskResult::Accepted);
    CHECK(r.check(OrderRequest{3, Side::Buy, 1000, 1}, 1000, kSec) == RiskResult::RateLimited);
    CHECK(r.check(OrderRequest{4, Side::Buy, 1000, 1}, 1000, kSec + kSec / 2) == RiskResult::Accepted);
}

TEST(risk_rejections_do_not_consume_rate_tokens) {
    RiskLimits l = loose_limits();
    l.max_orders_per_sec = 1;
    l.burst = 1;
    l.max_order_qty = 10;
    RiskChecker r(l);
    for (int i = 0; i < 5; ++i)
        CHECK(r.check(OrderRequest{1, Side::Buy, 1000, 11}, 1000, kSec) == RiskResult::MaxOrderQty);
    CHECK(r.check(OrderRequest{2, Side::Buy, 1000, 1}, 1000, kSec) == RiskResult::Accepted);
}

// ------------------------------------------------------------ encoding

TEST(encode_enter_order_layout) {
    std::byte buf[32] = {};
    std::size_t n = encode_enter_order(OrderRequest{0x0102030405060708ull, Side::Sell, 1500250, 300}, buf);
    CHECK_EQ(n, kEnterOrderSize);
    wire::Writer w;
    w.ch('O');
    w.u64(0x0102030405060708ull);
    w.ch('S');
    w.u32(300);
    w.u32(1500250);
    w.ch('I');
    CHECK_EQ(w.bytes.size(), kEnterOrderSize);
    CHECK(std::memcmp(buf, w.bytes.data(), n) == 0);
    std::byte small[5];
    CHECK_EQ(encode_enter_order(OrderRequest{1, Side::Buy, 1, 1}, small), 0u);
}

// ------------------------------------------------------------ strategy

TEST(strategy_needs_both_sides) {
    OrderBook b;
    ImbalanceStrategy s(StrategyParams{0.5, 10});
    CHECK_FALSE(s.on_book(b, 1).has_value());
    b.add(1, Side::Buy, 100, 1000);
    CHECK_FALSE(s.on_book(b, 1).has_value());
}

TEST(strategy_buys_on_bid_imbalance) {
    OrderBook b;
    ImbalanceStrategy s(StrategyParams{0.5, 10});
    b.add(1, Side::Buy, 100, 900);
    b.add(2, Side::Sell, 101, 100);  // imbalance (900-100)/1000 = 0.8
    auto o = s.on_book(b, 42);
    CHECK(o.has_value());
    CHECK(*o == (OrderRequest{42, Side::Buy, 101, 10}));  // lift the offer
}

TEST(strategy_sells_on_ask_imbalance) {
    OrderBook b;
    ImbalanceStrategy s(StrategyParams{0.5, 7});
    b.add(1, Side::Buy, 100, 100);
    b.add(2, Side::Sell, 101, 300);  // -0.5: exactly at threshold fires
    auto o = s.on_book(b, 1);
    CHECK(o.has_value());
    CHECK(*o == (OrderRequest{1, Side::Sell, 100, 7}));  // hit the bid
}

TEST(strategy_is_edge_triggered) {
    OrderBook b;
    ImbalanceStrategy s(StrategyParams{0.5, 1});
    b.add(1, Side::Buy, 100, 900);
    b.add(2, Side::Sell, 101, 100);
    CHECK(s.on_book(b, 1).has_value());
    b.add(3, Side::Buy, 100, 10);  // still imbalanced: no new order
    CHECK_FALSE(s.on_book(b, 2).has_value());
    b.add(4, Side::Sell, 101, 3000);  // flips to sell side
    auto o = s.on_book(b, 3);
    CHECK(o.has_value());
    CHECK(o->side == Side::Sell);
    b.remove(4);  // back to strong bid
    CHECK(s.on_book(b, 4).has_value());
    b.add(5, Side::Sell, 101, 810);  // 910 vs 910: neutral
    CHECK_FALSE(s.on_book(b, 5).has_value());
    b.remove(5);  // bid imbalance again after neutral: fires
    CHECK(s.on_book(b, 6).has_value());
}

// ------------------------------------------------------------ end to end

TEST(end_to_end_sends_order_on_signal) {
    TickToTrade t(Config{"AAPL", StrategyParams{0.6, 50}, loose_limits()});
    wire::Writer w;
    w.add_order(1, 1, 'S', 100, "AAPL", 1000100);
    w.add_order(2, 2, 'B', 100, "AAPL", 1000000);  // balanced: nothing
    w.add_order(3, 3, 'B', 900, "MSFT", 1000000);  // other stock: ignored
    t.on_bytes(view(w.bytes), kSec);
    CHECK(t.sent().empty());
    wire::Writer w2;
    w2.add_order(4, 4, 'B', 800, "AAPL", 1000000);  // bid 900 vs ask 100: buy
    t.on_bytes(view(w2.bytes), kSec + 1000);
    CHECK_EQ(t.sent().size(), 1u);
    CHECK(t.sent()[0] == (OrderRequest{1, Side::Buy, 1000100, 50}));
    CHECK_EQ(t.wire_out().size(), kEnterOrderSize);
    std::byte expect[kEnterOrderSize];
    encode_enter_order(t.sent()[0], expect);
    CHECK(std::memcmp(t.wire_out().data(), expect, kEnterOrderSize) == 0);
    CHECK_EQ(t.latency().count(), 1u);
    t.clear_wire_out();
    CHECK(t.wire_out().empty());
}

TEST(end_to_end_risk_blocks_after_fills) {
    RiskLimits l = loose_limits();
    l.max_position = 60;
    TickToTrade t(Config{"AAPL", StrategyParams{0.6, 50}, l});
    std::uint64_t ref = 1;
    auto pulse = [&](std::uint64_t ts) {
        // Create a strong bid imbalance, then neutralise it, so the
        // edge-triggered strategy fires once per pulse.
        wire::Writer w;
        std::uint64_t a = ref++, b = ref++;
        w.add_order(ts, a, 'S', 100, "AAPL", 1000100);
        w.add_order(ts, b, 'B', 900, "AAPL", 1000000);
        w.del(ts, b);
        w.del(ts, a);
        t.on_bytes(view(w.bytes), ts);
    };
    pulse(kSec);
    CHECK_EQ(t.sent().size(), 1u);
    t.on_fill(Side::Buy, 50);  // simulated exchange fill
    CHECK_EQ(t.position(), 50);
    pulse(2 * kSec);  // would take position to 100 > 60
    CHECK_EQ(t.sent().size(), 1u);
    CHECK_EQ(t.rejected().size(), 1u);
    CHECK(t.rejected()[0].second == RiskResult::MaxPosition);
    CHECK_EQ(t.latency().count(), 1u);  // latency is recorded for sent orders only
}

TEST(end_to_end_handles_split_messages) {
    TickToTrade t(Config{"AAPL", StrategyParams{0.6, 5}, loose_limits()});
    wire::Writer w;
    w.add_order(1, 1, 'S', 100, "AAPL", 1000100);
    w.add_order(2, 2, 'B', 900, "AAPL", 1000000);
    for (std::size_t i = 0; i < w.bytes.size(); ++i)
        t.on_bytes(std::span<const std::byte>(w.bytes.data() + i, 1), kSec + i);
    CHECK_EQ(t.sent().size(), 1u);
    CHECK_EQ(t.book().order_count(), 2u);
}

TEST(end_to_end_rate_limited_storm) {
    RiskLimits l = loose_limits();
    l.max_orders_per_sec = 5;
    l.burst = 5;
    TickToTrade t(Config{"AAPL", StrategyParams{0.6, 1}, l});
    // 100 signals within one millisecond: only the burst gets out.
    std::uint64_t ref = 1;
    for (int i = 0; i < 100; ++i) {
        wire::Writer w;
        std::uint64_t a = ref++, b = ref++;
        w.add_order(0, a, 'S', 100, "AAPL", 1000100);
        w.add_order(0, b, 'B', 900, "AAPL", 1000000);
        w.del(0, b);
        w.del(0, a);
        t.on_bytes(view(w.bytes), kSec + std::uint64_t(i) * 10'000);
    }
    CHECK_EQ(t.sent().size(), 5u);
    CHECK_EQ(t.rejected().size(), 95u);
    for (auto& [o, r] : t.rejected()) CHECK(r == RiskResult::RateLimited);
    // Client order ids are unique and increasing across sent and rejected.
    CHECK_EQ(t.sent()[4].client_order_id, 5u);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
