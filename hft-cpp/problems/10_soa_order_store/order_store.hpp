// Problem 10: Array-of-Structs vs Struct-of-Arrays order store
// Read README.md first. Implement everything marked TODO in BOTH classes.
// They have the same interface and must behave identically; only the
// memory layout differs.
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <unordered_map>  // fine for the id -> index lookup (or use your 08 map)
#include <vector>

namespace hft {

enum class Side : std::uint8_t { Buy, Sell };

// Returned by get(); not how you must store orders.
struct OrderView {
    std::uint64_t id;
    Side side;
    std::int64_t price;
    std::uint32_t qty;
    std::uint64_t timestamp;
};

// Array of Structs: one std::vector<Order> where Order holds every field.
// Use this realistic record (real order records carry more than the four
// fields we query): id, price, timestamp, account, qty, side, symbol[8],
// and some flags, about 64 bytes.
class AosOrderStore {
public:

    explicit AosOrderStore(std::size_t capacity = 0) {
        (void)capacity;
        // TODO: reserve storage
    }

    // Adds a resting order. Returns false if the id exists or qty == 0.
    bool add(std::uint64_t id, Side side, std::int64_t price, std::uint32_t qty, std::uint64_t ts) {
        (void)id; (void)side; (void)price; (void)qty; (void)ts;
        throw std::logic_error("TODO: AosOrderStore::add");
    }

    // O(1): swap the last order into the hole. Returns false if unknown.
    bool remove(std::uint64_t id) {
        (void)id;
        throw std::logic_error("TODO: AosOrderStore::remove");
    }

    // Sets a new quantity; qty == 0 removes the order. False if unknown.
    bool modify_qty(std::uint64_t id, std::uint32_t qty) {
        (void)id; (void)qty;
        throw std::logic_error("TODO: AosOrderStore::modify_qty");
    }

    std::optional<OrderView> get(std::uint64_t id) const {
        (void)id;
        throw std::logic_error("TODO: AosOrderStore::get");
    }

    std::size_t size() const noexcept { return 0; /* TODO */ }

    // ---- scans (the reason this problem exists) ----

    // Sum of qty over all orders on `side`.
    std::uint64_t total_qty(Side side) const {
        (void)side;
        throw std::logic_error("TODO: AosOrderStore::total_qty");
    }

    // Sum of qty of orders on `side` priced at `limit` or better
    // (buy: price >= limit, sell: price <= limit).
    std::uint64_t qty_at_or_better(Side side, std::int64_t limit) const {
        (void)side; (void)limit;
        throw std::logic_error("TODO: AosOrderStore::qty_at_or_better");
    }

    // Highest buy price / lowest sell price, nullopt if the side is empty.
    std::optional<std::int64_t> best_price(Side side) const {
        (void)side;
        throw std::logic_error("TODO: AosOrderStore::best_price");
    }

    // Number of orders (either side) with timestamp < ts.
    std::size_t count_older_than(std::uint64_t ts) const {
        (void)ts;
        throw std::logic_error("TODO: AosOrderStore::count_older_than");
    }

private:
    // TODO: std::vector<Order> plus an id -> index map.
};

// Struct of Arrays: one std::vector per field (ids, sides, prices, qtys,
// timestamps), all indexed by the same position.
class SoaOrderStore {
public:

    explicit SoaOrderStore(std::size_t capacity = 0) {
        (void)capacity;
        // TODO: reserve storage
    }

    // Adds a resting order. Returns false if the id exists or qty == 0.
    bool add(std::uint64_t id, Side side, std::int64_t price, std::uint32_t qty, std::uint64_t ts) {
        (void)id; (void)side; (void)price; (void)qty; (void)ts;
        throw std::logic_error("TODO: SoaOrderStore::add");
    }

    // O(1): swap the last order into the hole. Returns false if unknown.
    bool remove(std::uint64_t id) {
        (void)id;
        throw std::logic_error("TODO: SoaOrderStore::remove");
    }

    // Sets a new quantity; qty == 0 removes the order. False if unknown.
    bool modify_qty(std::uint64_t id, std::uint32_t qty) {
        (void)id; (void)qty;
        throw std::logic_error("TODO: SoaOrderStore::modify_qty");
    }

    std::optional<OrderView> get(std::uint64_t id) const {
        (void)id;
        throw std::logic_error("TODO: SoaOrderStore::get");
    }

    std::size_t size() const noexcept { return 0; /* TODO */ }

    // ---- scans (the reason this problem exists) ----

    // Sum of qty over all orders on `side`.
    std::uint64_t total_qty(Side side) const {
        (void)side;
        throw std::logic_error("TODO: SoaOrderStore::total_qty");
    }

    // Sum of qty of orders on `side` priced at `limit` or better
    // (buy: price >= limit, sell: price <= limit).
    std::uint64_t qty_at_or_better(Side side, std::int64_t limit) const {
        (void)side; (void)limit;
        throw std::logic_error("TODO: SoaOrderStore::qty_at_or_better");
    }

    // Highest buy price / lowest sell price, nullopt if the side is empty.
    std::optional<std::int64_t> best_price(Side side) const {
        (void)side;
        throw std::logic_error("TODO: SoaOrderStore::best_price");
    }

    // Number of orders (either side) with timestamp < ts.
    std::size_t count_older_than(std::uint64_t ts) const {
        (void)ts;
        throw std::logic_error("TODO: SoaOrderStore::count_older_than");
    }

private:
    // TODO: one vector per field plus an id -> index map.
};

}  // namespace hft
