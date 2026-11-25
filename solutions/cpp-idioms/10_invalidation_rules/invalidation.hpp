// Reference solution for problem 10.
#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace idioms {

struct Order {
    int id;
    bool cancelled;
};

inline void remove_cancelled(std::vector<Order>& orders) {
    orders.erase(std::remove_if(orders.begin(), orders.end(), [](const Order& o) { return o.cancelled; }),
                 orders.end());
}

inline void duplicate_evens(std::vector<int>& v) {
    for (auto it = v.begin(); it != v.end();) {
        if (*it % 2 == 0) {
            it = v.insert(it, *it);  // `it` -> the copy; original now at it + 1
            it += 2;
        } else {
            ++it;
        }
    }
}

struct Quote {
    double price;
    long expires_at;
};

inline std::size_t expire(std::map<int, Quote>& book, long now) {
    std::size_t erased = 0;
    for (auto it = book.begin(); it != book.end();) {
        if (it->second.expires_at <= now) {
            it = book.erase(it);
            ++erased;
        } else {
            ++it;
        }
    }
    return erased;
}

inline void append_self(std::vector<std::string>& v) {
    const std::size_t n = v.size();
    v.reserve(2 * n);
    std::copy_n(v.begin(), n, std::back_inserter(v));
}

inline int best_then_grow(std::vector<int>& v, const std::vector<int>& extra) {
    int best = *std::max_element(v.begin(), v.end());  // copy, not a reference
    for (int x : extra) {
        v.push_back(x);
        best = std::max(best, x);
    }
    return best;
}

}  // namespace idioms
