// Problem 13: the algorithm toolkit. No raw loops.
// Read README.md first. Implement everything marked TODO.
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
    (void)prices;
    (void)k;
    throw std::logic_error("TODO: top_k");
}

inline long percentile(std::vector<long> latencies, double p) {
    (void)latencies;
    (void)p;
    throw std::logic_error("TODO: percentile");
}

struct Trade {
    double price;
    long qty;
};

inline double vwap(const std::vector<Trade>& trades) {
    (void)trades;
    throw std::logic_error("TODO: vwap");
}

struct Order {
    int id;
    bool urgent;
};

inline void urgent_first(std::vector<Order>& orders) {
    (void)orders;
    throw std::logic_error("TODO: urgent_first");
}

inline long first_gap(const std::vector<double>& prices, double max_jump) {
    (void)prices;
    (void)max_jump;
    throw std::logic_error("TODO: first_gap");
}

inline void move_to_front(std::vector<int>& ids, int id) {
    (void)ids;
    (void)id;
    throw std::logic_error("TODO: move_to_front");
}

inline std::vector<long> running_position(const std::vector<long>& fills) {
    (void)fills;
    throw std::logic_error("TODO: running_position");
}

inline void dedupe_sorted(std::vector<int>& ids) {
    (void)ids;
    throw std::logic_error("TODO: dedupe_sorted");
}

}  // namespace idioms
