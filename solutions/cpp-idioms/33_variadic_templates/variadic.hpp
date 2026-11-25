// Reference solution for problem 33.
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
    return (first + ... + rest);
}

namespace detail {
template <class... Ts>
struct all_same : std::true_type {};
template <class First, class... Rest>
struct all_same<First, Rest...> : std::bool_constant<(std::is_same_v<First, Rest> && ...)> {};
}  // namespace detail

template <class... Ts>
inline constexpr bool all_same_v = detail::all_same<Ts...>::value;

template <class... Args>
std::string join_args(const char* sep, const Args&... args) {
    std::ostringstream os;
    bool first = true;
    ((os << (first ? "" : sep) << args, first = false), ...);
    return os.str();
}

template <class Tuple, class F>
void for_each_in_tuple(Tuple&& t, F&& f) {
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (f(std::get<I>(t)), ...);
    }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
}

template <class... Ts, class F>
auto transform_tuple(const std::tuple<Ts...>& t, F&& f) {
    return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return std::tuple{f(std::get<I>(t))...};
    }(std::index_sequence_for<Ts...>{});
}

template <std::size_t N, class F>
constexpr auto make_table(F f) {
    return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return std::array<decltype(f(std::size_t{0})), N>{f(I)...};
    }(std::make_index_sequence<N>{});
}

template <class... Ts>
struct TypeList {};

template <class L>
inline constexpr std::size_t list_size_v = 0;
template <class... Ts>
inline constexpr std::size_t list_size_v<TypeList<Ts...>> = sizeof...(Ts);

template <class T, class L>
inline constexpr bool contains_v = false;
template <class T, class... Ts>
inline constexpr bool contains_v<T, TypeList<Ts...>> = (std::is_same_v<T, Ts> || ...);

namespace detail {
template <class T, class L>
struct index_of;
template <class T, class... Rest>
struct index_of<T, TypeList<T, Rest...>> : std::integral_constant<std::size_t, 0> {};
template <class T, class Head, class... Rest>
struct index_of<T, TypeList<Head, Rest...>>
    : std::integral_constant<std::size_t, 1 + index_of<T, TypeList<Rest...>>::value> {};
}  // namespace detail

template <class T, class L>
inline constexpr std::size_t index_of_v = detail::index_of<T, L>::value;

template <std::size_t N, class L>
struct at;
template <class Head, class... Rest>
struct at<0, TypeList<Head, Rest...>> {
    using type = Head;
};
template <std::size_t N, class Head, class... Rest>
struct at<N, TypeList<Head, Rest...>> : at<N - 1, TypeList<Rest...>> {};
template <std::size_t N, class L>
using at_t = typename at<N, L>::type;

}  // namespace idioms
