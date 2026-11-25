// Problem 20 (capstone): tick-to-trade
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
//
// Reuses your solutions to 09, 15 and 17.
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../09_latency_histogram/latency_histogram.hpp"
#include "../15_itch_parser/itch_parser.hpp"
#include "../17_l2_order_book/order_book.hpp"

namespace hft::t2t {

using book::Side;

struct OrderRequest {
    std::uint64_t client_order_id;
    Side side;
    std::int64_t price;
    std::uint32_t qty;
    friend bool operator==(const OrderRequest&, const OrderRequest&) = default;
};

// ------------------------------------------------------------------ rate

// Token bucket in integer arithmetic. Starts full with `burst` tokens and
// refills at `rate_per_sec`, never above `burst`. Time is passed in (ns), so
// it is deterministic and testable.
class TokenBucket {
public:
    TokenBucket(std::uint32_t rate_per_sec, std::uint32_t burst) {
        (void)rate_per_sec;
        (void)burst;
    }

    // Takes one token if available at time now_ns.
    bool try_consume(std::uint64_t now_ns) {
        (void)now_ns;
        throw std::logic_error("TODO: try_consume");
    }

    // Whole tokens available at time now_ns.
    std::uint32_t available(std::uint64_t now_ns) {
        (void)now_ns;
        throw std::logic_error("TODO: available");
    }
};

// ------------------------------------------------------------------ risk

struct RiskLimits {
    std::uint32_t max_order_qty;
    std::int64_t max_position;  // |position after a full fill| must stay <= this
    std::uint32_t max_orders_per_sec;
    std::uint32_t burst;
    std::int64_t price_collar;  // max |order price - reference price|
};

enum class RiskResult { Accepted, MaxOrderQty, PriceCollar, NoReferencePrice, MaxPosition, RateLimited };

// Pre-trade risk checks, applied in this order:
//   qty == 0 or qty > max_order_qty      -> MaxOrderQty
//   no reference price                   -> NoReferencePrice
//   |price - reference| > price_collar   -> PriceCollar
//   position + signed qty out of limits  -> MaxPosition
//   no rate token                        -> RateLimited
// A rate token is consumed only by orders that pass everything else.
class RiskChecker {
public:
    explicit RiskChecker(RiskLimits limits) { (void)limits; }

    RiskResult check(const OrderRequest& o, std::optional<std::int64_t> reference, std::uint64_t now_ns) {
        (void)o; (void)reference; (void)now_ns;
        throw std::logic_error("TODO: check");
    }

    // Position changes only when the exchange reports a fill.
    void on_fill(Side side, std::uint32_t qty) {
        (void)side; (void)qty;
        throw std::logic_error("TODO: on_fill");
    }
    std::int64_t position() const { throw std::logic_error("TODO: position"); }
};

// ------------------------------------------------------------------ wire

// "Enter Order" message, 19 bytes, big-endian:
//   [0] 'O'  [1..8] client_order_id  [9] 'B'/'S'  [10..13] qty
//   [14..17] price (uint32)  [18] 'I' (IOC)
inline constexpr std::size_t kEnterOrderSize = 19;

// Returns bytes written, or 0 if out is too small.
inline std::size_t encode_enter_order(const OrderRequest& o, std::span<std::byte> out) {
    (void)o;
    (void)out;
    return 0;  // TODO
}

// ------------------------------------------------------------------ strategy

struct StrategyParams {
    double threshold;    // in (0, 1]
    std::uint32_t clip;  // order size
};

// Top-of-book imbalance: I = (bid_qty - ask_qty) / (bid_qty + ask_qty).
// Signal +1 if I >= threshold, -1 if I <= -threshold, else 0 (also 0 when a
// side is empty). Edge triggered: returns an order only when the signal
// becomes non-zero and differs from the previous call's signal.
//   +1: buy `clip` at the best ask.   -1: sell `clip` at the best bid.
class ImbalanceStrategy {
public:
    explicit ImbalanceStrategy(StrategyParams p) { (void)p; }

    std::optional<OrderRequest> on_book(const book::OrderBook& b, std::uint64_t next_id) {
        (void)b; (void)next_id;
        throw std::logic_error("TODO: on_book");
    }
};

// ------------------------------------------------------------------ engine

struct Config {
    std::string stock;
    StrategyParams strategy;
    RiskLimits risk;
};

// Market data in, orders out, on one thread:
//   bytes -> itch::parse -> BookBuilder/OrderBook -> after EVERY message:
//   strategy -> (if an order) risk check -> encode into wire_out().
// Client order ids start at 1 and increase for every order the strategy
// produces (accepted or rejected). Reference price for the collar is the
// mid, (best_bid + best_ask) / 2, if both sides exist.
class TickToTrade {
public:
    explicit TickToTrade(Config cfg) { (void)cfg; }

    // A chunk of the ITCH stream (messages may be split across chunks).
    // rx_ts_ns is the logical receive time, used for rate limiting. Record
    // into latency() the time from entering on_bytes to finishing the
    // encode of each SENT order, measured with steady_now_ns().
    void on_bytes(std::span<const std::byte> chunk, std::uint64_t rx_ts_ns) {
        (void)chunk; (void)rx_ts_ns;
        throw std::logic_error("TODO: on_bytes");
    }

    // The (simulated) exchange reports a fill.
    void on_fill(Side side, std::uint32_t qty) {
        (void)side; (void)qty;
        throw std::logic_error("TODO: on_fill");
    }

    const std::vector<std::byte>& wire_out() const { throw std::logic_error("TODO: wire_out"); }
    void clear_wire_out() { throw std::logic_error("TODO: clear_wire_out"); }
    const std::vector<OrderRequest>& sent() const { throw std::logic_error("TODO: sent"); }
    const std::vector<std::pair<OrderRequest, RiskResult>>& rejected() const {
        throw std::logic_error("TODO: rejected");
    }
    const LatencyHistogram& latency() const { throw std::logic_error("TODO: latency"); }
    std::int64_t position() const { throw std::logic_error("TODO: position"); }
    const book::OrderBook& book() const { throw std::logic_error("TODO: book"); }
};

}  // namespace hft::t2t
