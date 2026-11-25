// Problem 14: the binary search family and heterogeneous lookup.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
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
    (void)levels;
    (void)price;
    throw std::logic_error("TODO: find_level");
}

inline long qty_between(const std::vector<Level>& levels, double lo, double hi) {
    (void)levels;
    (void)lo;
    (void)hi;
    throw std::logic_error("TODO: qty_between");
}

inline std::size_t count_of(const std::vector<int>& sorted, int x) {
    (void)sorted;
    (void)x;
    throw std::logic_error("TODO: count_of");
}

inline std::size_t insert_sorted(std::vector<int>& v, int x) {
    (void)v;
    (void)x;
    throw std::logic_error("TODO: insert_sorted");
}

struct ChildOrder {
    int id;
    bool filled;
};

inline std::size_t first_unfilled(const std::vector<ChildOrder>& orders) {
    (void)orders;
    throw std::logic_error("TODO: first_unfilled");
}

class SymbolTable {
public:
    void add(std::string symbol, int id) {
        (void)symbol;
        (void)id;
        throw std::logic_error("TODO: SymbolTable::add");
    }
    std::optional<int> find(std::string_view symbol) const {
        (void)symbol;
        throw std::logic_error("TODO: SymbolTable::find");
    }

private:
    std::map<std::string, int> table_;  // TODO: make lookups by string_view allocation-free
};

}  // namespace idioms
