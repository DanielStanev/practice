// Reference solution for problem 16.
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

namespace detail {
template <class R>
auto collect(R&& r) {
    std::vector<std::ranges::range_value_t<R>> out;
    std::ranges::copy(r, std::back_inserter(out));
    return out;
}
}  // namespace detail

struct Trade {
    std::string symbol;
    long qty;
};

inline std::vector<std::string> symbols_traded(const std::vector<Trade>& trades, long min_qty) {
    return detail::collect(trades | std::views::filter([min_qty](const Trade& t) { return t.qty >= min_qty; }) |
                           std::views::transform(&Trade::symbol));
}

inline std::vector<std::string_view> split_fields(std::string_view line, char sep) {
    std::vector<std::string_view> out;
    for (auto sub : line | std::views::split(sep)) out.emplace_back(sub.begin(), sub.end());
    return out;
}

inline std::vector<int> last_even(const std::vector<int>& v, std::size_t n) {
    return detail::collect(v | std::views::reverse | std::views::filter([](int x) { return x % 2 == 0; }) |
                           std::views::take(n));
}

inline std::vector<double> returns(const std::vector<double>& prices) {
    if (prices.empty()) return {};
    return detail::collect(std::views::iota(std::size_t{1}, prices.size()) |
                           std::views::transform([&prices](std::size_t i) { return prices[i] / prices[i - 1] - 1.0; }));
}

inline long total_position(const std::map<std::string, long>& book) {
    auto values = book | std::views::values;
    return std::accumulate(values.begin(), values.end(), 0L);
}

inline std::vector<int> all_fill_ids(const std::vector<std::vector<int>>& batches) {
    return detail::collect(batches | std::views::join);
}

struct Event {
    long time;
    double price;
};

inline std::vector<double> first_after_open(const std::vector<Event>& events, long open_time) {
    return detail::collect(events | std::views::drop_while([open_time](const Event& e) { return e.time < open_time; }) |
                           std::views::take_while([](const Event& e) { return e.price > 0; }) |
                           std::views::transform(&Event::price));
}

}  // namespace idioms
