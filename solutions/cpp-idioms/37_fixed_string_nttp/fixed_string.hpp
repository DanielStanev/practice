// Reference solution for problem 37.
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace idioms {

template <std::size_t N>
struct FixedString {
    char data[N]{};
    constexpr FixedString(const char (&s)[N]) { std::copy_n(s, N, data); }
    constexpr std::size_t size() const { return N - 1; }
    constexpr std::string_view view() const { return {data, N - 1}; }
    bool operator==(const FixedString&) const = default;
};

namespace detail {
constexpr std::uint32_t fnv1a(std::string_view s) {
    std::uint32_t h = 2166136261u;
    for (char c : s) {
        h ^= static_cast<unsigned char>(c);
        h *= 16777619u;
    }
    return h;
}
}  // namespace detail

template <FixedString Name, class T>
struct Field {
    static constexpr std::string_view name = Name.view();
    static constexpr std::uint32_t id = detail::fnv1a(Name.view());
    T value{};
};

template <class T, std::size_t Cap>
class StaticVector {
public:
    constexpr void push_back(const T& v) {
        if (size_ == Cap) throw std::length_error("StaticVector full");
        data_[size_++] = v;
    }
    constexpr void pop_back() { --size_; }
    constexpr std::size_t size() const { return size_; }
    static constexpr std::size_t capacity() { return Cap; }
    constexpr T& operator[](std::size_t i) { return data_[i]; }
    constexpr const T& operator[](std::size_t i) const { return data_[i]; }
    constexpr T& back() { return data_[size_ - 1]; }
    constexpr auto begin() { return data_.begin(); }
    constexpr auto end() { return data_.begin() + static_cast<std::ptrdiff_t>(size_); }
    constexpr auto begin() const { return data_.begin(); }
    constexpr auto end() const { return data_.begin() + static_cast<std::ptrdiff_t>(size_); }

private:
    std::array<T, Cap> data_{};
    std::size_t size_ = 0;
};

template <std::size_t N>
constexpr std::array<std::string_view, N> make_symbol_table(std::array<std::string_view, N> names) {
    std::sort(names.begin(), names.end());
    if (std::adjacent_find(names.begin(), names.end()) != names.end()) throw std::invalid_argument("duplicate symbol");
    return names;
}

template <std::size_t N>
constexpr int symbol_index(const std::array<std::string_view, N>& table, std::string_view name) {
    auto it = std::lower_bound(table.begin(), table.end(), name);
    return (it != table.end() && *it == name) ? static_cast<int>(it - table.begin()) : -1;
}

}  // namespace idioms
