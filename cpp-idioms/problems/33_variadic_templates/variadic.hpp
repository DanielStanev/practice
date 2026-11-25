// Problem 33: variadic templates, folds, index sequences, type lists.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <array>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace idioms {

template <class T, class... Ts>
auto sum(T first, Ts... rest) {
    ((void)rest, ...);
    throw std::logic_error("TODO: sum");
    return first;
}

template <class... Ts>
inline constexpr bool all_same_v = false;  // TODO

template <class... Args>
std::string join_args(const char* sep, const Args&... args) {
    (void)sep;
    ((void)args, ...);
    throw std::logic_error("TODO: join_args");
}

template <class Tuple, class F>
void for_each_in_tuple(Tuple&& t, F&& f) {
    (void)t;
    (void)f;
    throw std::logic_error("TODO: for_each_in_tuple");
}

template <class... Ts, class F>
auto transform_tuple(const std::tuple<Ts...>& t, F&& f) {
    (void)t;
    (void)f;
    throw std::logic_error("TODO: transform_tuple");
    return std::tuple<std::decay_t<std::invoke_result_t<F&, const Ts&>>...>{};  // the type to return
}

template <std::size_t N, class F>
constexpr auto make_table(F f) {
    (void)f;
    return std::array<decltype(f(std::size_t{0})), N>{};  // TODO: {f(0), ..., f(N-1)}
}

template <class... Ts>
struct TypeList {};

// TODO: all four metafunctions.
template <class L>
inline constexpr std::size_t list_size_v = 0;

template <class T, class L>
inline constexpr bool contains_v = false;

template <class T, class L>
inline constexpr std::size_t index_of_v = static_cast<std::size_t>(-1);

template <std::size_t N, class L>
struct at {
    using type = void;
};
template <std::size_t N, class L>
using at_t = typename at<N, L>::type;

}  // namespace idioms
