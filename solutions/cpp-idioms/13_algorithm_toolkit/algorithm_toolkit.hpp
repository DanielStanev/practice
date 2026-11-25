// Reference solution for problem 13.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace idioms {

inline std::vector<double> top_k(std::vector<double> prices, std::size_t k) {
    k = std::min(k, prices.size());
    std::partial_sort(prices.begin(), prices.begin() + static_cast<std::ptrdiff_t>(k), prices.end(),
                      std::greater<>{});
    prices.resize(k);
    return prices;
}

inline long percentile(std::vector<long> latencies, double p) {
    const auto idx = static_cast<std::ptrdiff_t>(std::floor(p * static_cast<double>(latencies.size() - 1)));
    std::nth_element(latencies.begin(), latencies.begin() + idx, latencies.end());
    return latencies[static_cast<std::size_t>(idx)];
}

struct Trade {
    double price;
    long qty;
};

inline double vwap(const std::vector<Trade>& trades) {
    const double notional = std::transform_reduce(trades.begin(), trades.end(), 0.0, std::plus<>{},
                                                  [](const Trade& t) { return t.price * static_cast<double>(t.qty); });
    const long qty = std::transform_reduce(trades.begin(), trades.end(), 0L, std::plus<>{},
                                           [](const Trade& t) { return t.qty; });
    return qty == 0 ? 0.0 : notional / static_cast<double>(qty);
}

struct Order {
    int id;
    bool urgent;
};

inline void urgent_first(std::vector<Order>& orders) {
    std::stable_partition(orders.begin(), orders.end(), [](const Order& o) { return o.urgent; });
}

inline long first_gap(const std::vector<double>& prices, double max_jump) {
    auto it = std::adjacent_find(prices.begin(), prices.end(),
                                 [max_jump](double a, double b) { return std::abs(b - a) > max_jump; });
    return it == prices.end() ? -1 : static_cast<long>(it - prices.begin());
}

inline void move_to_front(std::vector<int>& ids, int id) {
    auto it = std::find(ids.begin(), ids.end(), id);
    if (it != ids.end()) std::rotate(ids.begin(), it, std::next(it));
}

inline std::vector<long> running_position(const std::vector<long>& fills) {
    std::vector<long> out(fills.size());
    std::inclusive_scan(fills.begin(), fills.end(), out.begin());
    return out;
}

inline void dedupe_sorted(std::vector<int>& ids) { ids.erase(std::unique(ids.begin(), ids.end()), ids.end()); }

}  // namespace idioms
