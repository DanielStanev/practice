// Problem 18: projections, borrowed ranges, dangling.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace idioms {

struct Order {
    int id;
    double price;
    long time;
    long qty;
};

inline void sort_book(std::vector<Order>& orders) {
    (void)orders;
    throw std::logic_error("TODO: sort_book");
}

inline const Order* largest_order(const std::vector<Order>& orders) {
    (void)orders;
    throw std::logic_error("TODO: largest_order");
}

inline const Order* find_by_id(const std::vector<Order>& orders, int id) {
    (void)orders;
    (void)id;
    throw std::logic_error("TODO: find_by_id");
}

inline std::pair<long, long> qty_extremes(const std::vector<Order>& orders) {
    (void)orders;
    throw std::logic_error("TODO: qty_extremes");
}

struct IntSpan {
    const int* data = nullptr;
    std::size_t size = 0;
    const int* begin() const { return data; }
    const int* end() const { return data + size; }
};

// TODO: make IntSpan a borrowed range. The specialisation of
// std::ranges::enable_borrowed_range must be written outside namespace idioms
// (close the namespace, specialise, reopen it).

inline long index_of_first_negative(IntSpan s) {
    (void)s;
    throw std::logic_error("TODO: index_of_first_negative");
}

}  // namespace idioms
