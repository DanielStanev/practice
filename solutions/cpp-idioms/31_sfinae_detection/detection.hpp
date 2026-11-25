// Reference solution for problem 31.
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

template <class T, class = void>
struct has_size : std::false_type {};
template <class T>
struct has_size<T, std::void_t<decltype(std::declval<const T&>().size())>> : std::true_type {};
template <class T>
inline constexpr bool has_size_v = has_size<T>::value;

template <class T, class = void>
struct has_reserve : std::false_type {};
template <class T>
struct has_reserve<T, std::void_t<decltype(std::declval<T&>().reserve(std::size_t{}))>> : std::true_type {};
template <class T>
inline constexpr bool has_reserve_v = has_reserve<T>::value;

template <class T, class = void>
struct has_mapped_type : std::false_type {};
template <class T>
struct has_mapped_type<T, std::void_t<typename T::mapped_type>> : std::true_type {};
template <class T>
inline constexpr bool has_mapped_type_v = has_mapped_type<T>::value;

template <class T, class = void>
struct is_streamable : std::false_type {};
template <class T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<const T&>())>>
    : std::true_type {};
template <class T>
inline constexpr bool is_streamable_v = is_streamable<T>::value;

template <class C, std::enable_if_t<has_size_v<C>, int> = 0>
std::size_t container_size(const C& c) {
    return static_cast<std::size_t>(c.size());
}

template <class C, std::enable_if_t<!has_size_v<C>, int> = 0>
std::size_t container_size(const C& c) {
    return static_cast<std::size_t>(std::distance(std::begin(c), std::end(c)));
}

template <class T>
std::string to_text(const T& x) {
    if constexpr (std::is_same_v<T, std::string>) {
        return x;
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(x);
    } else if constexpr (is_streamable_v<T>) {
        std::ostringstream os;
        os << x;
        return os.str();
    } else {
        return "<?>";
    }
}

template <class Dst, class Src>
void append_all(Dst& dst, const Src& src) {
    if constexpr (has_reserve_v<Dst>) dst.reserve(dst.size() + container_size(src));
    for (const auto& x : src) dst.insert(dst.end(), x);
}

}  // namespace idioms
