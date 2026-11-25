// Reference solution for problem 18.
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
    std::ranges::sort(orders, {}, [](const Order& o) { return std::tuple(-o.price, o.time, o.id); });
}

inline const Order* largest_order(const std::vector<Order>& orders) {
    auto it = std::ranges::max_element(orders, {}, &Order::qty);
    return it == orders.end() ? nullptr : &*it;
}

inline const Order* find_by_id(const std::vector<Order>& orders, int id) {
    auto it = std::ranges::find(orders, id, &Order::id);
    return it == orders.end() ? nullptr : &*it;
}

inline std::pair<long, long> qty_extremes(const std::vector<Order>& orders) {
    auto [lo, hi] = std::ranges::minmax(orders | std::views::transform(&Order::qty));
    return {lo, hi};
}

struct IntSpan {
    const int* data = nullptr;
    std::size_t size = 0;
    const int* begin() const { return data; }
    const int* end() const { return data + size; }
};

}  // namespace idioms

template <>
inline constexpr bool std::ranges::enable_borrowed_range<idioms::IntSpan> = true;

namespace idioms {

inline long index_of_first_negative(IntSpan s) {
    const int* it = std::ranges::find_if(IntSpan{s}, [](int x) { return x < 0; });  // temporary: still fine
    return it == s.end() ? -1 : static_cast<long>(it - s.begin());
}

}  // namespace idioms
