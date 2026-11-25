#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

namespace hft::match {

enum class Side : std::uint8_t { Buy, Sell };
enum class TimeInForce : std::uint8_t { GTC, IOC };
enum class Status : std::uint8_t { Accepted, RejectedDuplicateId, RejectedInvalidQty };

struct Trade {
    std::uint64_t taker_id;
    std::uint64_t maker_id;
    std::int64_t price;
    std::uint32_t qty;
    friend bool operator==(const Trade&, const Trade&) = default;
};

struct SubmitResult {
    Status status;
    std::uint32_t filled;
    std::uint32_t resting;
};

class MatchingEngine {
    struct Order {
        std::uint64_t id;
        std::uint32_t qty;
    };
    using Queue = std::list<Order>;
    struct Loc {
        Side side;
        std::int64_t price;
        Queue::iterator it;
    };

public:
    explicit MatchingEngine(std::size_t expected_orders = 1024) { index_.reserve(expected_orders); }

    SubmitResult submit(std::uint64_t id, Side side, std::int64_t price, std::uint32_t qty,
                        TimeInForce tif = TimeInForce::GTC) {
        if (qty == 0) return {Status::RejectedInvalidQty, 0, 0};
        if (index_.count(id)) return {Status::RejectedDuplicateId, 0, 0};
        std::uint32_t left = qty;
        if (side == Side::Buy) left = match(id, left, asks_, [price](std::int64_t p) { return p <= price; });
        else left = match(id, left, bids_, [price](std::int64_t p) { return p >= price; });
        std::uint32_t resting = 0;
        if (left > 0 && tif == TimeInForce::GTC) {
            rest(id, side, price, left);
            resting = left;
        }
        return {Status::Accepted, qty - left, resting};
    }

    SubmitResult submit_market(std::uint64_t id, Side side, std::uint32_t qty) {
        std::int64_t px = side == Side::Buy ? std::numeric_limits<std::int64_t>::max()
                                            : std::numeric_limits<std::int64_t>::min();
        return submit(id, side, px, qty, TimeInForce::IOC);
    }

    bool cancel(std::uint64_t id) {
        auto it = index_.find(id);
        if (it == index_.end()) return false;
        erase_order(it->second);
        index_.erase(it);
        return true;
    }

    bool modify(std::uint64_t id, std::int64_t new_price, std::uint32_t new_qty) {
        auto it = index_.find(id);
        if (it == index_.end() || new_qty == 0) return false;
        Loc loc = it->second;
        if (new_price == loc.price && new_qty <= loc.it->qty) {
            loc.it->qty = new_qty;  // keeps priority
            return true;
        }
        erase_order(loc);
        index_.erase(it);
        submit(id, loc.side, new_price, new_qty, TimeInForce::GTC);
        return true;
    }

    std::optional<std::int64_t> best_bid() const {
        if (bids_.empty()) return std::nullopt;
        return bids_.begin()->first;
    }
    std::optional<std::int64_t> best_ask() const {
        if (asks_.empty()) return std::nullopt;
        return asks_.begin()->first;
    }

    std::uint64_t volume_at(Side side, std::int64_t price) const {
        auto sum = [](const Queue& q) {
            std::uint64_t s = 0;
            for (auto& o : q) s += o.qty;
            return s;
        };
        if (side == Side::Buy) {
            auto it = bids_.find(price);
            return it == bids_.end() ? 0 : sum(it->second);
        }
        auto it = asks_.find(price);
        return it == asks_.end() ? 0 : sum(it->second);
    }

    std::size_t resting_orders() const { return index_.size(); }

    std::optional<std::uint32_t> resting_qty(std::uint64_t id) const {
        auto it = index_.find(id);
        if (it == index_.end()) return std::nullopt;
        return it->second.it->qty;
    }

    const std::vector<Trade>& trades() const { return trades_; }
    void clear_trades() { trades_.clear(); }

private:
    template <typename Book, typename Crosses>
    std::uint32_t match(std::uint64_t taker, std::uint32_t left, Book& book, Crosses crosses) {
        while (left > 0 && !book.empty()) {
            auto lvl = book.begin();
            if (!crosses(lvl->first)) break;
            Queue& q = lvl->second;
            while (left > 0 && !q.empty()) {
                Order& maker = q.front();
                std::uint32_t fill = std::min(left, maker.qty);
                trades_.push_back(Trade{taker, maker.id, lvl->first, fill});
                left -= fill;
                maker.qty -= fill;
                if (maker.qty == 0) {
                    index_.erase(maker.id);
                    q.pop_front();
                }
            }
            if (q.empty()) book.erase(lvl);
        }
        return left;
    }

    void rest(std::uint64_t id, Side side, std::int64_t price, std::uint32_t qty) {
        Queue& q = side == Side::Buy ? bids_[price] : asks_[price];
        q.push_back(Order{id, qty});
        index_.emplace(id, Loc{side, price, std::prev(q.end())});
    }

    void erase_order(const Loc& loc) {
        auto apply = [&](auto& book) {
            auto lvl = book.find(loc.price);
            lvl->second.erase(loc.it);
            if (lvl->second.empty()) book.erase(lvl);
        };
        if (loc.side == Side::Buy) apply(bids_);
        else apply(asks_);
    }

    std::map<std::int64_t, Queue, std::greater<>> bids_;
    std::map<std::int64_t, Queue, std::less<>> asks_;
    std::unordered_map<std::uint64_t, Loc> index_;
    std::vector<Trade> trades_;
};

}  // namespace hft::match
