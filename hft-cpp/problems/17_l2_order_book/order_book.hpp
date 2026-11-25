// Problem 17: price-level (L2) order book built from order-level (L3) events
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

// Builds on problem 15: BookBuilder consumes its decoded messages.
#include "../15_itch_parser/itch_parser.hpp"

namespace hft::book {

enum class Side : std::uint8_t { Buy, Sell };

// One aggregated price level.
struct Level {
    std::int64_t price;
    std::uint64_t qty;      // total remaining quantity at this price
    std::uint32_t orders;   // number of orders at this price
    friend bool operator==(const Level&, const Level&) = default;
};

class OrderBook {
public:
    // New resting order. False if ref already exists or qty == 0.
    bool add(std::uint64_t ref, Side side, std::int64_t price, std::uint32_t qty) {
        (void)ref; (void)side; (void)price; (void)qty;
        throw std::logic_error("TODO: add");
    }

    // Reduce an order by qty (an execution or a partial cancel). If qty >=
    // the remaining quantity the order is removed. False if ref is unknown.
    bool execute(std::uint64_t ref, std::uint32_t qty) {
        (void)ref; (void)qty;
        throw std::logic_error("TODO: execute");
    }
    bool cancel(std::uint64_t ref, std::uint32_t qty) {
        (void)ref; (void)qty;
        throw std::logic_error("TODO: cancel");
    }

    // Remove the whole order. False if unknown.
    bool remove(std::uint64_t ref) {
        (void)ref;
        throw std::logic_error("TODO: remove");
    }

    // Replace orig with a new order new_ref (same side, new qty and price).
    // False, with no change, if orig is unknown or new_ref is already used.
    bool replace(std::uint64_t orig, std::uint64_t new_ref, std::uint32_t qty, std::int64_t price) {
        (void)orig; (void)new_ref; (void)qty; (void)price;
        throw std::logic_error("TODO: replace");
    }

    // Highest bid / lowest ask level, or nullopt if that side is empty.
    std::optional<Level> best_bid() const { throw std::logic_error("TODO: best_bid"); }
    std::optional<Level> best_ask() const { throw std::logic_error("TODO: best_ask"); }

    // Number of non-empty price levels on a side.
    std::size_t depth(Side side) const {
        (void)side;
        throw std::logic_error("TODO: depth");
    }

    // Writes up to out.size() levels, best first. Returns how many.
    std::size_t levels(Side side, std::span<Level> out) const {
        (void)side; (void)out;
        throw std::logic_error("TODO: levels");
    }

    // Total quantity at an exact price (0 if no level).
    std::uint64_t qty_at(Side side, std::int64_t price) const {
        (void)side; (void)price;
        throw std::logic_error("TODO: qty_at");
    }

    std::size_t order_count() const { throw std::logic_error("TODO: order_count"); }

    // Remaining quantity of one order.
    std::optional<std::uint32_t> order_qty(std::uint64_t ref) const {
        (void)ref;
        throw std::logic_error("TODO: order_qty");
    }

private:
    // TODO: order lookup by ref, plus aggregated levels per side.
};

// An itch::Handler that applies messages to an OrderBook. Add Orders for
// other stocks are ignored; the other messages refer to orders by ref, and
// refs the book does not know are ignored.
class BookBuilder {
public:
    // stock is space padded to 8 characters to match the wire format.
    BookBuilder(OrderBook& book, std::string_view stock) : book_(book) {
        (void)stock;
        // TODO
    }

    void on_add(const itch::AddOrder& m) {
        (void)m;
        throw std::logic_error("TODO: on_add");
    }
    void on_execute(const itch::OrderExecuted& m) {
        (void)m;
        throw std::logic_error("TODO: on_execute");
    }
    void on_cancel(const itch::OrderCancel& m) {
        (void)m;
        throw std::logic_error("TODO: on_cancel");
    }
    void on_delete(const itch::OrderDelete& m) {
        (void)m;
        throw std::logic_error("TODO: on_delete");
    }
    void on_replace(const itch::OrderReplace& m) {
        (void)m;
        throw std::logic_error("TODO: on_replace");
    }

private:
    OrderBook& book_;
};

}  // namespace hft::book
