// Problem 18: limit order matching engine (price-time priority)
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace hft::match {

enum class Side : std::uint8_t { Buy, Sell };
enum class TimeInForce : std::uint8_t {
    GTC,  // good till cancel: any unfilled remainder rests in the book
    IOC,  // immediate or cancel: any unfilled remainder is discarded
};
enum class Status : std::uint8_t { Accepted, RejectedDuplicateId, RejectedInvalidQty };

struct Trade {
    std::uint64_t taker_id;  // the incoming (aggressive) order
    std::uint64_t maker_id;  // the resting (passive) order
    std::int64_t price;      // always the maker's price
    std::uint32_t qty;
    friend bool operator==(const Trade&, const Trade&) = default;
};

struct SubmitResult {
    Status status;
    std::uint32_t filled;   // quantity traded immediately
    std::uint32_t resting;  // quantity left in the book (0 for IOC)
};

class MatchingEngine {
public:
    // expected_orders: capacity hint so you can pre-allocate.
    explicit MatchingEngine(std::size_t expected_orders = 1024) {
        (void)expected_orders;
    }

    // New limit order. Matches against the opposite side while prices
    // cross, best price first and oldest first within a price, then rests
    // any remainder (GTC) or discards it (IOC). Appends a Trade per fill.
    // Rejects qty == 0 and ids that are currently resting.
    SubmitResult submit(std::uint64_t id, Side side, std::int64_t price, std::uint32_t qty,
                        TimeInForce tif = TimeInForce::GTC) {
        (void)id; (void)side; (void)price; (void)qty; (void)tif;
        throw std::logic_error("TODO: submit");
    }

    // Market order: IOC at any price.
    SubmitResult submit_market(std::uint64_t id, Side side, std::uint32_t qty) {
        (void)id; (void)side; (void)qty;
        throw std::logic_error("TODO: submit_market");
    }

    // Removes a resting order. False if not resting.
    bool cancel(std::uint64_t id) {
        (void)id;
        throw std::logic_error("TODO: cancel");
    }

    // Changes a resting order. Same price and qty not increased: update in
    // place, keeping time priority. Otherwise: the order loses priority and
    // is re-submitted (GTC) with the new price/qty, which may trade.
    // False if not resting or new_qty == 0.
    bool modify(std::uint64_t id, std::int64_t new_price, std::uint32_t new_qty) {
        (void)id; (void)new_price; (void)new_qty;
        throw std::logic_error("TODO: modify");
    }

    std::optional<std::int64_t> best_bid() const { throw std::logic_error("TODO: best_bid"); }
    std::optional<std::int64_t> best_ask() const { throw std::logic_error("TODO: best_ask"); }

    // Total resting quantity at a price.
    std::uint64_t volume_at(Side side, std::int64_t price) const {
        (void)side; (void)price;
        throw std::logic_error("TODO: volume_at");
    }

    std::size_t resting_orders() const { throw std::logic_error("TODO: resting_orders"); }

    std::optional<std::uint32_t> resting_qty(std::uint64_t id) const {
        (void)id;
        throw std::logic_error("TODO: resting_qty");
    }

    // Every trade since construction or the last clear_trades(), in order.
    const std::vector<Trade>& trades() const { return trades_; }
    void clear_trades() { trades_.clear(); }

private:
    std::vector<Trade> trades_;
    // TODO: price levels per side, each a FIFO of orders, plus id lookup
    // that lets cancel find an order in O(1).
};

}  // namespace hft::match
