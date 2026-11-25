// Problem 38: lambdas and captures.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace idioms {

inline auto make_counter(int start) {
    (void)start;
    return []() -> int { throw std::logic_error("TODO: make_counter"); };
}

inline std::vector<std::function<int(int)>> make_multipliers(int n) {
    (void)n;
    throw std::logic_error("TODO: make_multipliers");
}

template <class F, class... Fs>
auto compose(F f, Fs... fs) {
    ((void)fs, ...);
    return [f](auto&& x) -> decltype(f(x)) { throw std::logic_error("TODO: compose"); };
}

template <class F>
auto memoize(F f) {
    (void)f;
    return [](long) -> long { throw std::logic_error("TODO: memoize"); };
}

inline auto make_owner(std::unique_ptr<int> p) {
    (void)p;
    return []() -> int { throw std::logic_error("TODO: make_owner"); };
}

struct Order {
    int id;
    double price;
    std::function<std::string()> printer() const {
        throw std::logic_error("TODO: printer");
    }
};

inline double tick_size(std::string_view venue, double price) {
    (void)venue;
    (void)price;
    throw std::logic_error("TODO: tick_size");
}

}  // namespace idioms
