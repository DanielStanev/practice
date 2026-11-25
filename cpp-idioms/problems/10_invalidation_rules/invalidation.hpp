// Problem 10: iterator invalidation.
// Read README.md first. Implement everything marked TODO.
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
    (void)orders;
    throw std::logic_error("TODO: remove_cancelled");
}

inline void duplicate_evens(std::vector<int>& v) {
    (void)v;
    throw std::logic_error("TODO: duplicate_evens");
}

struct Quote {
    double price;
    long expires_at;
};

inline std::size_t expire(std::map<int, Quote>& book, long now) {
    (void)book;
    (void)now;
    throw std::logic_error("TODO: expire");
}

inline void append_self(std::vector<std::string>& v) {
    (void)v;
    throw std::logic_error("TODO: append_self");
}

// The obvious version is below. It is wrong: `best` refers into `v`, and
// push_back may reallocate, so the final read is a use-after-free.
//
//   int& best = *std::max_element(v.begin(), v.end());
//   for (int x : extra) v.push_back(x);
//   return std::max(best, *std::max_element(extra.begin(), extra.end()));
//
// Write a correct version.
inline int best_then_grow(std::vector<int>& v, const std::vector<int>& extra) {
    (void)v;
    (void)extra;
    throw std::logic_error("TODO: best_then_grow");
}

}  // namespace idioms
