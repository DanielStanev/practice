// Problem 16: ranges and views.
// Read README.md first. Implement everything marked TODO with view pipelines.
#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <map>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace idioms {

struct Trade {
    std::string symbol;
    long qty;
};

inline std::vector<std::string> symbols_traded(const std::vector<Trade>& trades, long min_qty) {
    (void)trades;
    (void)min_qty;
    throw std::logic_error("TODO: symbols_traded");
}

inline std::vector<std::string_view> split_fields(std::string_view line, char sep) {
    (void)line;
    (void)sep;
    throw std::logic_error("TODO: split_fields");
}

inline std::vector<int> last_even(const std::vector<int>& v, std::size_t n) {
    (void)v;
    (void)n;
    throw std::logic_error("TODO: last_even");
}

inline std::vector<double> returns(const std::vector<double>& prices) {
    (void)prices;
    throw std::logic_error("TODO: returns");
}

inline long total_position(const std::map<std::string, long>& book) {
    (void)book;
    throw std::logic_error("TODO: total_position");
}

inline std::vector<int> all_fill_ids(const std::vector<std::vector<int>>& batches) {
    (void)batches;
    throw std::logic_error("TODO: all_fill_ids");
}

struct Event {
    long time;
    double price;
};

inline std::vector<double> first_after_open(const std::vector<Event>& events, long open_time) {
    (void)events;
    (void)open_time;
    throw std::logic_error("TODO: first_after_open");
}

}  // namespace idioms
