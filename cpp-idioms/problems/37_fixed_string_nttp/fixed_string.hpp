// Problem 37: FixedString NTTPs and constexpr containers.
// Read README.md first. Implement everything marked TODO.
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
    constexpr FixedString(const char (&s)[N]) {
        (void)s;  // TODO: copy
    }
    constexpr std::size_t size() const { return 0; }            // TODO
    constexpr std::string_view view() const { return {}; }      // TODO
    bool operator==(const FixedString&) const = default;
};

template <FixedString Name, class T>
struct Field {
    static constexpr std::string_view name = {};   // TODO
    static constexpr std::uint32_t id = 0;         // TODO: FNV-1a of name
    T value{};
};

template <class T, std::size_t Cap>
class StaticVector {
public:
    constexpr void push_back(const T& v) {
        (void)v;
        throw std::logic_error("TODO: push_back");
    }
    constexpr void pop_back() { throw std::logic_error("TODO: pop_back"); }
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
    throw std::logic_error("TODO: make_symbol_table");
    return names;
}

template <std::size_t N>
constexpr int symbol_index(const std::array<std::string_view, N>& table, std::string_view name) {
    (void)table;
    (void)name;
    throw std::logic_error("TODO: symbol_index");
    return -1;
}

}  // namespace idioms
