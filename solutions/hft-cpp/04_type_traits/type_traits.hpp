#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

namespace hft::meta {

// ---------------------------------------------------------------- basics
template <typename A, typename B>
struct is_same {
    static constexpr bool value = false;
};
template <typename A>
struct is_same<A, A> {
    static constexpr bool value = true;
};
template <typename A, typename B>
inline constexpr bool is_same_v = is_same<A, B>::value;

template <typename T>
struct remove_cvref {
    using type = T;
};
template <typename T>
struct remove_cvref<const T> : remove_cvref<T> {};
template <typename T>
struct remove_cvref<volatile T> : remove_cvref<T> {};
template <typename T>
struct remove_cvref<const volatile T> : remove_cvref<T> {};
template <typename T>
struct remove_cvref<T&> : remove_cvref<T> {};
template <typename T>
struct remove_cvref<T&&> : remove_cvref<T> {};
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <bool B, typename T, typename F>
struct conditional {
    using type = T;
};
template <typename T, typename F>
struct conditional<false, T, F> {
    using type = F;
};
template <bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

// ------------------------------------------------------------- type lists
template <typename... Ts>
struct TypeList {};

template <typename List>
struct list_size;
template <typename... Ts>
struct list_size<TypeList<Ts...>> {
    static constexpr std::size_t value = sizeof...(Ts);
};
template <typename List>
inline constexpr std::size_t list_size_v = list_size<List>::value;

template <std::size_t I, typename List>
struct type_at {
    using type = void;
};
template <typename Head, typename... Tail>
struct type_at<0, TypeList<Head, Tail...>> {
    using type = Head;
};
template <std::size_t I, typename Head, typename... Tail>
struct type_at<I, TypeList<Head, Tail...>> {
    using type = typename type_at<I - 1, TypeList<Tail...>>::type;
};
template <std::size_t I, typename List>
using type_at_t = typename type_at<I, List>::type;

template <typename T, typename List>
struct index_of;
template <typename T, typename... Ts>
struct index_of<T, TypeList<Ts...>> {
    static constexpr int value = [] {
        constexpr bool matches[] = {is_same_v<T, Ts>..., false};
        for (std::size_t i = 0; i < sizeof...(Ts); ++i)
            if (matches[i]) return static_cast<int>(i);
        return -1;
    }();
};
template <typename T, typename List>
inline constexpr int index_of_v = index_of<T, List>::value;

template <typename T, typename List>
inline constexpr bool contains_v = index_of_v<T, List> >= 0;

template <typename T, typename List>
struct push_front;
template <typename T, typename... Ts>
struct push_front<T, TypeList<Ts...>> {
    using type = TypeList<T, Ts...>;
};
template <typename T, typename List>
using push_front_t = typename push_front<T, List>::type;

template <typename List>
struct max_size;
template <typename... Ts>
struct max_size<TypeList<Ts...>> {
    static constexpr std::size_t value = [] {
        std::size_t m = 0;
        ((m = sizeof(Ts) > m ? sizeof(Ts) : m), ...);
        return m;
    }();
};
template <typename List>
inline constexpr std::size_t max_size_v = max_size<List>::value;

// ------------------------------------------------------- constexpr utils
constexpr bool is_power_of_two(std::uint64_t x) { return x != 0 && (x & (x - 1)) == 0; }

constexpr std::uint64_t next_power_of_two(std::uint64_t x) {
    if (x <= 1) return 1;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x + 1;
}

constexpr int log2_floor(std::uint64_t x) {
    int r = -1;
    while (x) {
        x >>= 1;
        ++r;
    }
    return r;
}

template <std::size_t N>
constexpr std::array<std::uint64_t, N> pow10_table() {
    std::array<std::uint64_t, N> t{};
    std::uint64_t v = 1;
    for (std::size_t i = 0; i < N; ++i) {
        t[i] = v;
        v *= 10;
    }
    return t;
}

constexpr int count_digits(std::uint64_t x) {
    constexpr auto p = pow10_table<20>();
    int d = 1;
    while (d < 20 && x >= p[d]) ++d;
    return d;
}

constexpr std::uint64_t fnv1a(std::string_view s) {
    std::uint64_t h = 14695981039346656037ull;
    for (char c : s) {
        h ^= static_cast<unsigned char>(c);
        h *= 1099511628211ull;
    }
    return h;
}

// --------------------------------------------------------------- concepts
template <typename T>
concept WireMessage = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> &&
                      requires {
                          { T::kType } -> std::convertible_to<char>;
                      };

template <typename List>
struct dispatcher;
template <typename... Ms>
struct dispatcher<TypeList<Ms...>> {
    template <typename F>
    static constexpr bool run(char type, F&& f) {
        return ((type == Ms::kType ? (f(std::type_identity<Ms>{}), true) : false) || ...);
    }
};

template <typename List, typename F>
constexpr bool dispatch(char type, F&& f) {
    return dispatcher<List>::run(type, std::forward<F>(f));
}

}  // namespace hft::meta
