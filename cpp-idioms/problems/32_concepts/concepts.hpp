// Problem 32: concepts.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <concepts>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

namespace idioms {

enum class Side { buy, sell };

template <class T>
concept Price = false;  // TODO

template <class B>
concept Book = false;  // TODO

// TODO: constrain with Book.
template <class B>
std::optional<typename B::price_type> spread(const B& b) {
    (void)b;
    throw std::logic_error("TODO: spread");
}

// TODO: replace with the overload set.
template <class T>
std::string_view describe(const T&) {
    throw std::logic_error("TODO: describe");
}

// TODO: constrain on Price.
template <class P>
struct Level {
    P price;
    long qty;
};

// TODO: constrain (input range whose value type is a Price).
template <class R>
std::ranges::range_value_t<R> total(R&& r) {
    (void)r;
    throw std::logic_error("TODO: total");
}

}  // namespace idioms
