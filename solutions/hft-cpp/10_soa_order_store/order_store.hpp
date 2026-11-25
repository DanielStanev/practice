#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <unordered_map>
#include <vector>

namespace hft {

enum class Side : std::uint8_t { Buy, Sell };

struct OrderView {
    std::uint64_t id;
    Side side;
    std::int64_t price;
    std::uint32_t qty;
    std::uint64_t timestamp;
};

namespace detail {
inline bool at_or_better(Side side, std::int64_t price, std::int64_t limit) {
    return side == Side::Buy ? price >= limit : price <= limit;
}
}  // namespace detail

class AosOrderStore {
    struct Order {
        std::uint64_t id;
        std::int64_t price;
        std::uint64_t timestamp;
        std::uint64_t account = 0;
        std::uint32_t qty;
        Side side;
        char symbol[8] = {};
        char flags[11] = {};
    };

public:
    explicit AosOrderStore(std::size_t capacity = 0) {
        orders_.reserve(capacity);
        index_.reserve(capacity);
    }

    bool add(std::uint64_t id, Side side, std::int64_t price, std::uint32_t qty, std::uint64_t ts) {
        if (qty == 0 || index_.count(id)) return false;
        index_.emplace(id, orders_.size());
        Order o{};
        o.id = id;
        o.price = price;
        o.timestamp = ts;
        o.qty = qty;
        o.side = side;
        orders_.push_back(o);
        return true;
    }

    bool remove(std::uint64_t id) {
        auto it = index_.find(id);
        if (it == index_.end()) return false;
        std::size_t i = it->second;
        index_.erase(it);
        if (i != orders_.size() - 1) {
            orders_[i] = orders_.back();
            index_[orders_[i].id] = i;
        }
        orders_.pop_back();
        return true;
    }

    bool modify_qty(std::uint64_t id, std::uint32_t qty) {
        if (qty == 0) return remove(id);
        auto it = index_.find(id);
        if (it == index_.end()) return false;
        orders_[it->second].qty = qty;
        return true;
    }

    std::optional<OrderView> get(std::uint64_t id) const {
        auto it = index_.find(id);
        if (it == index_.end()) return std::nullopt;
        const Order& o = orders_[it->second];
        return OrderView{o.id, o.side, o.price, o.qty, o.timestamp};
    }

    std::size_t size() const noexcept { return orders_.size(); }

    std::uint64_t total_qty(Side side) const {
        std::uint64_t sum = 0;
        for (const auto& o : orders_) sum += o.qty * std::uint64_t(o.side == side);
        return sum;
    }

    std::uint64_t qty_at_or_better(Side side, std::int64_t limit) const {
        std::uint64_t sum = 0;
        for (const auto& o : orders_)
            sum += o.qty * (std::uint64_t(o.side == side) & std::uint64_t(detail::at_or_better(side, o.price, limit)));
        return sum;
    }

    std::optional<std::int64_t> best_price(Side side) const {
        std::optional<std::int64_t> best;
        for (const auto& o : orders_)
            if (o.side == side && (!best || detail::at_or_better(side, o.price, *best) )) best = o.price;
        return best;
    }

    std::size_t count_older_than(std::uint64_t ts) const {
        std::size_t n = 0;
        for (const auto& o : orders_) n += o.timestamp < ts;
        return n;
    }

private:
    std::vector<Order> orders_;
    std::unordered_map<std::uint64_t, std::size_t> index_;
};

class SoaOrderStore {
public:
    explicit SoaOrderStore(std::size_t capacity = 0) {
        ids_.reserve(capacity);
        sides_.reserve(capacity);
        prices_.reserve(capacity);
        qtys_.reserve(capacity);
        timestamps_.reserve(capacity);
        index_.reserve(capacity);
    }

    bool add(std::uint64_t id, Side side, std::int64_t price, std::uint32_t qty, std::uint64_t ts) {
        if (qty == 0 || index_.count(id)) return false;
        index_.emplace(id, ids_.size());
        ids_.push_back(id);
        sides_.push_back(side);
        prices_.push_back(price);
        qtys_.push_back(qty);
        timestamps_.push_back(ts);
        return true;
    }

    bool remove(std::uint64_t id) {
        auto it = index_.find(id);
        if (it == index_.end()) return false;
        std::size_t i = it->second;
        index_.erase(it);
        std::size_t last = ids_.size() - 1;
        if (i != last) {
            ids_[i] = ids_[last];
            sides_[i] = sides_[last];
            prices_[i] = prices_[last];
            qtys_[i] = qtys_[last];
            timestamps_[i] = timestamps_[last];
            index_[ids_[i]] = i;
        }
        ids_.pop_back();
        sides_.pop_back();
        prices_.pop_back();
        qtys_.pop_back();
        timestamps_.pop_back();
        return true;
    }

    bool modify_qty(std::uint64_t id, std::uint32_t qty) {
        if (qty == 0) return remove(id);
        auto it = index_.find(id);
        if (it == index_.end()) return false;
        qtys_[it->second] = qty;
        return true;
    }

    std::optional<OrderView> get(std::uint64_t id) const {
        auto it = index_.find(id);
        if (it == index_.end()) return std::nullopt;
        std::size_t i = it->second;
        return OrderView{ids_[i], sides_[i], prices_[i], qtys_[i], timestamps_[i]};
    }

    std::size_t size() const noexcept { return ids_.size(); }

    std::uint64_t total_qty(Side side) const {
        std::uint64_t sum = 0;
        const std::size_t n = ids_.size();
        for (std::size_t i = 0; i < n; ++i) sum += qtys_[i] * std::uint64_t(sides_[i] == side);
        return sum;
    }

    std::uint64_t qty_at_or_better(Side side, std::int64_t limit) const {
        std::uint64_t sum = 0;
        const std::size_t n = ids_.size();
        // Branch-free bodies: the compiler can vectorise these loops.
        if (side == Side::Buy) {
            for (std::size_t i = 0; i < n; ++i) {
                std::uint64_t take = std::uint64_t(sides_[i] == side) & std::uint64_t(prices_[i] >= limit);
                sum += qtys_[i] * take;
            }
        } else {
            for (std::size_t i = 0; i < n; ++i) {
                std::uint64_t take = std::uint64_t(sides_[i] == side) & std::uint64_t(prices_[i] <= limit);
                sum += qtys_[i] * take;
            }
        }
        return sum;
    }

    std::optional<std::int64_t> best_price(Side side) const {
        bool found = false;
        std::int64_t best = side == Side::Buy ? std::numeric_limits<std::int64_t>::min()
                                              : std::numeric_limits<std::int64_t>::max();
        const std::size_t n = ids_.size();
        for (std::size_t i = 0; i < n; ++i) {
            if (sides_[i] != side) continue;
            found = true;
            best = side == Side::Buy ? std::max(best, prices_[i]) : std::min(best, prices_[i]);
        }
        if (!found) return std::nullopt;
        return best;
    }

    std::size_t count_older_than(std::uint64_t ts) const {
        std::size_t c = 0;
        for (auto t : timestamps_) c += t < ts;
        return c;
    }

private:
    std::vector<std::uint64_t> ids_;
    std::vector<Side> sides_;
    std::vector<std::int64_t> prices_;
    std::vector<std::uint32_t> qtys_;
    std::vector<std::uint64_t> timestamps_;
    std::unordered_map<std::uint64_t, std::size_t> index_;
};

}  // namespace hft
