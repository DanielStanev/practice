// Problem 31: SFINAE and the detection idiom (no concepts or requires!).
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstddef>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace idioms {

// TODO: detect T::size()
template <class T, class = void>
struct has_size : std::false_type {};
template <class T>
inline constexpr bool has_size_v = has_size<T>::value;

// TODO: detect T::reserve(std::size_t)
template <class T, class = void>
struct has_reserve : std::false_type {};
template <class T>
inline constexpr bool has_reserve_v = has_reserve<T>::value;

// TODO: detect T::mapped_type
template <class T, class = void>
struct has_mapped_type : std::false_type {};
template <class T>
inline constexpr bool has_mapped_type_v = has_mapped_type<T>::value;

// TODO: detect std::ostream& << const T&
template <class T, class = void>
struct is_streamable : std::false_type {};
template <class T>
inline constexpr bool is_streamable_v = is_streamable<T>::value;

// TODO: two overloads selected with std::enable_if_t.
template <class C>
std::size_t container_size(const C& c) {
    (void)c;
    throw std::logic_error("TODO: container_size");
}

template <class T>
std::string to_text(const T& x) {
    (void)x;
    throw std::logic_error("TODO: to_text");
}

template <class Dst, class Src>
void append_all(Dst& dst, const Src& src) {
    (void)dst;
    (void)src;
    throw std::logic_error("TODO: append_all");
}

}  // namespace idioms
