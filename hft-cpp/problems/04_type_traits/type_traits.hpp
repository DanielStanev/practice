// Problem 04: compile-time type traits and constexpr utilities
// Read README.md first. Replace every TODO placeholder with a real
// implementation. Placeholders compile but give wrong answers.
//
// Rules: implement is_same, remove_cvref, conditional and the TypeList
// algorithms yourself (partial specialisation, recursion, fold expressions).
// You may use std::is_trivially_copyable / std::is_standard_layout /
// std::type_identity / std::convertible_to from the standard library.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

namespace hft::meta {

// ---------------------------------------------------------------- basics

// value is true iff A and B are exactly the same type.
template <typename A, typename B>
struct is_same {
    static constexpr bool value = false;  // TODO
};
template <typename A, typename B>
inline constexpr bool is_same_v = is_same<A, B>::value;

// Strips top-level const/volatile and references.
template <typename T>
struct remove_cvref {
    using type = T;  // TODO
};
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

// type is T if B is true, otherwise F.
template <bool B, typename T, typename F>
struct conditional {
    using type = T;  // TODO
};
template <bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

// ------------------------------------------------------------- type lists
template <typename... Ts>
struct TypeList {};

// Number of types in the list.
template <typename List>
inline constexpr std::size_t list_size_v = 0;  // TODO

// The I-th type of the list.
template <std::size_t I, typename List>
struct type_at {
    using type = void;  // TODO
};
template <std::size_t I, typename List>
using type_at_t = typename type_at<I, List>::type;

// Index of the first occurrence of T in List, or -1.
template <typename T, typename List>
inline constexpr int index_of_v = -1;  // TODO

template <typename T, typename List>
inline constexpr bool contains_v = false;  // TODO

// TypeList<T, Ts...> from TypeList<Ts...>.
template <typename T, typename List>
struct push_front {
    using type = List;  // TODO
};
template <typename T, typename List>
using push_front_t = typename push_front<T, List>::type;

// Largest sizeof() of the types in the list (0 for an empty list).
template <typename List>
inline constexpr std::size_t max_size_v = 0;  // TODO

// ------------------------------------------------------- constexpr utils
// All of these must be usable in constant expressions.

constexpr bool is_power_of_two(std::uint64_t x) {
    (void)x;
    return false;  // TODO
}

// Smallest power of two >= x. next_power_of_two(0) == 1.
constexpr std::uint64_t next_power_of_two(std::uint64_t x) {
    (void)x;
    return 0;  // TODO
}

// floor(log2(x)) for x > 0.
constexpr int log2_floor(std::uint64_t x) {
    (void)x;
    return 0;  // TODO
}

// {1, 10, 100, ...} with N entries, computed at compile time.
template <std::size_t N>
constexpr std::array<std::uint64_t, N> pow10_table() {
    return {};  // TODO
}

// Number of decimal digits in x (count_digits(0) == 1).
constexpr int count_digits(std::uint64_t x) {
    (void)x;
    return 0;  // TODO
}

// 64-bit FNV-1a hash.
constexpr std::uint64_t fnv1a(std::string_view s) {
    (void)s;
    return 0;  // TODO
}

// --------------------------------------------------------------- concepts

// A message type that can be memcpy'd to/from the wire: trivially copyable,
// standard layout, and has a static `kType` convertible to char.
template <typename T>
concept WireMessage = false;  // TODO

// Calls f(std::type_identity<M>{}) for the first M in List with
// M::kType == type. Returns whether a match was found. Must be constexpr.
template <typename List, typename F>
constexpr bool dispatch(char type, F&& f) {
    (void)type;
    (void)f;
    return false;  // TODO
}

}  // namespace hft::meta
