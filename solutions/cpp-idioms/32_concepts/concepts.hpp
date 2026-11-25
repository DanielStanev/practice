// Reference solution for problem 32.
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
concept Price = std::regular<T> && std::totally_ordered<T> && requires(T a, T b) {
    { a + b } -> std::same_as<T>;
    { a - b } -> std::same_as<T>;
};

template <class B>
concept Book = requires { typename B::price_type; } && Price<typename B::price_type> &&
               requires(B& b, const B& cb, typename B::price_type p, long q) {
                   { cb.best_bid() } -> std::convertible_to<std::optional<typename B::price_type>>;
                   { cb.best_ask() } -> std::convertible_to<std::optional<typename B::price_type>>;
                   b.add(Side::buy, p, q);
               };

template <Book B>
std::optional<typename B::price_type> spread(const B& b) {
    std::optional<typename B::price_type> bid = b.best_bid();
    std::optional<typename B::price_type> ask = b.best_ask();
    if (!bid || !ask) return std::nullopt;
    return *ask - *bid;
}

template <class T>
std::string_view describe(const T&) {
    return "other";
}
template <Price T>
std::string_view describe(const T&) {
    return "price";
}
template <class T>
    requires Price<T> && std::integral<T>
std::string_view describe(const T&) {
    return "integral";
}
template <class T>
    requires Price<T> && std::signed_integral<T>
std::string_view describe(const T&) {
    return "signed integral";
}
template <class T>
    requires Price<T> && std::floating_point<T>
std::string_view describe(const T&) {
    return "floating";
}

template <Price P>
struct Level {
    P price;
    long qty;
};

template <std::ranges::input_range R>
    requires Price<std::ranges::range_value_t<R>>
auto total(R&& r) {
    std::ranges::range_value_t<R> sum{};
    for (auto&& x : r) sum = sum + x;
    return sum;
}

}  // namespace idioms
