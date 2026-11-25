// Reference solution for problem 14.
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace idioms {

struct Level {
    double price;
    long qty;
};

inline const Level* find_level(const std::vector<Level>& levels, double price) {
    auto it = std::lower_bound(levels.begin(), levels.end(), price,
                               [](const Level& l, double p) { return l.price < p; });
    return (it != levels.end() && it->price == price) ? &*it : nullptr;
}

inline long qty_between(const std::vector<Level>& levels, double lo, double hi) {
    auto first = std::ranges::lower_bound(levels, lo, {}, &Level::price);
    auto last = std::ranges::upper_bound(levels, hi, {}, &Level::price);
    if (last < first) return 0;
    return std::accumulate(first, last, 0L, [](long acc, const Level& l) { return acc + l.qty; });
}

inline std::size_t count_of(const std::vector<int>& sorted, int x) {
    auto [first, last] = std::equal_range(sorted.begin(), sorted.end(), x);
    return static_cast<std::size_t>(last - first);
}

inline std::size_t insert_sorted(std::vector<int>& v, int x) {
    auto it = v.insert(std::upper_bound(v.begin(), v.end(), x), x);
    return static_cast<std::size_t>(it - v.begin());
}

struct ChildOrder {
    int id;
    bool filled;
};

inline std::size_t first_unfilled(const std::vector<ChildOrder>& orders) {
    auto it = std::partition_point(orders.begin(), orders.end(), [](const ChildOrder& o) { return o.filled; });
    return static_cast<std::size_t>(it - orders.begin());
}

class SymbolTable {
public:
    void add(std::string symbol, int id) { table_.insert_or_assign(std::move(symbol), id); }
    std::optional<int> find(std::string_view symbol) const {
        auto it = table_.find(symbol);
        if (it == table_.end()) return std::nullopt;
        return it->second;
    }

private:
    std::map<std::string, int, std::less<>> table_;
};

}  // namespace idioms
