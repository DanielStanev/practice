#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string_view>
#include <unordered_map>

#include "../15_itch_parser/itch_parser.hpp"

namespace hft::book {

enum class Side : std::uint8_t { Buy, Sell };

struct Level {
    std::int64_t price;
    std::uint64_t qty;
    std::uint32_t orders;
    friend bool operator==(const Level&, const Level&) = default;
};

class OrderBook {
    struct Order {
        Side side;
        std::int64_t price;
        std::uint32_t qty;
    };
    struct Agg {
        std::uint64_t qty = 0;
        std::uint32_t orders = 0;
    };

public:
    bool add(std::uint64_t ref, Side side, std::int64_t price, std::uint32_t qty) {
        if (qty == 0) return false;
        if (!orders_.emplace(ref, Order{side, price, qty}).second) return false;
        Agg& a = side == Side::Buy ? bids_[price] : asks_[price];
        a.qty += qty;
        a.orders += 1;
        return true;
    }

    bool execute(std::uint64_t ref, std::uint32_t qty) { return reduce(ref, qty); }
    bool cancel(std::uint64_t ref, std::uint32_t qty) { return reduce(ref, qty); }

    bool remove(std::uint64_t ref) {
        auto it = orders_.find(ref);
        if (it == orders_.end()) return false;
        take_from_level(it->second, it->second.qty, true);
        orders_.erase(it);
        return true;
    }

    bool replace(std::uint64_t orig, std::uint64_t new_ref, std::uint32_t qty, std::int64_t price) {
        auto it = orders_.find(orig);
        if (it == orders_.end() || (new_ref != orig && orders_.count(new_ref))) return false;
        Side side = it->second.side;
        remove(orig);
        return add(new_ref, side, price, qty);
    }

    std::optional<Level> best_bid() const {
        if (bids_.empty()) return std::nullopt;
        auto& [p, a] = *bids_.begin();
        return Level{p, a.qty, a.orders};
    }
    std::optional<Level> best_ask() const {
        if (asks_.empty()) return std::nullopt;
        auto& [p, a] = *asks_.begin();
        return Level{p, a.qty, a.orders};
    }

    std::size_t depth(Side side) const { return side == Side::Buy ? bids_.size() : asks_.size(); }

    std::size_t levels(Side side, std::span<Level> out) const {
        std::size_t n = 0;
        auto fill = [&](const auto& m) {
            for (auto& [p, a] : m) {
                if (n == out.size()) break;
                out[n++] = Level{p, a.qty, a.orders};
            }
        };
        if (side == Side::Buy) fill(bids_);
        else fill(asks_);
        return n;
    }

    std::uint64_t qty_at(Side side, std::int64_t price) const {
        if (side == Side::Buy) {
            auto it = bids_.find(price);
            return it == bids_.end() ? 0 : it->second.qty;
        }
        auto it = asks_.find(price);
        return it == asks_.end() ? 0 : it->second.qty;
    }

    std::size_t order_count() const { return orders_.size(); }

    std::optional<std::uint32_t> order_qty(std::uint64_t ref) const {
        auto it = orders_.find(ref);
        if (it == orders_.end()) return std::nullopt;
        return it->second.qty;
    }

private:
    bool reduce(std::uint64_t ref, std::uint32_t qty) {
        auto it = orders_.find(ref);
        if (it == orders_.end()) return false;
        Order& o = it->second;
        if (qty >= o.qty) {
            take_from_level(o, o.qty, true);
            orders_.erase(it);
        } else {
            take_from_level(o, qty, false);
            o.qty -= qty;
        }
        return true;
    }

    void take_from_level(const Order& o, std::uint32_t qty, bool whole_order) {
        auto apply = [&](auto& m) {
            auto lit = m.find(o.price);
            lit->second.qty -= qty;
            if (whole_order && --lit->second.orders == 0) m.erase(lit);
        };
        if (o.side == Side::Buy) apply(bids_);
        else apply(asks_);
    }

    std::unordered_map<std::uint64_t, Order> orders_;
    std::map<std::int64_t, Agg, std::greater<>> bids_;
    std::map<std::int64_t, Agg, std::less<>> asks_;
};

// Applies ITCH messages to an OrderBook, keeping only one stock's orders.
class BookBuilder {
public:
    BookBuilder(OrderBook& book, std::string_view stock) : book_(book) {
        stock_.fill(' ');
        for (std::size_t i = 0; i < stock.size() && i < 8; ++i) stock_[i] = stock[i];
    }

    void on_add(const itch::AddOrder& m) {
        if (m.stock != stock_) return;
        book_.add(m.order_ref, m.side == itch::Side::Buy ? Side::Buy : Side::Sell, m.price, m.shares);
    }
    void on_execute(const itch::OrderExecuted& m) { book_.execute(m.order_ref, m.executed_shares); }
    void on_cancel(const itch::OrderCancel& m) { book_.cancel(m.order_ref, m.cancelled_shares); }
    void on_delete(const itch::OrderDelete& m) { book_.remove(m.order_ref); }
    void on_replace(const itch::OrderReplace& m) {
        book_.replace(m.orig_order_ref, m.new_order_ref, m.shares, m.price);
    }

private:
    OrderBook& book_;
    std::array<char, 8> stock_;
};

}  // namespace hft::book
