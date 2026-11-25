// Reference solution for problem 40.
#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace idioms {

namespace detail {
inline std::string_view first_word_impl(std::string_view s) {
    auto start = s.find_first_not_of(' ');
    if (start == std::string_view::npos) return {};
    auto end = s.find(' ', start);
    return s.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);
}
}  // namespace detail

inline std::string_view first_word(const std::string& s) { return detail::first_word_impl(s); }
inline std::string_view first_word(const char* s) { return detail::first_word_impl(s); }
std::string_view first_word(std::string&&) = delete;

class Instrument {
public:
    explicit Instrument(std::string s) : symbol_(std::move(s)) {}
    const std::string& symbol() const& { return symbol_; }
    std::string symbol() && { return std::move(symbol_); }

private:
    std::string symbol_;
};

class Tag {
public:
    explicit Tag(std::string_view name) : name_(name) {}
    std::string_view name() const { return name_; }

private:
    std::string name_;  // own the characters
};

// Return by value: one copy, can never dangle. (The alternative, a const&
// overload for lvalues plus by-value overloads for rvalues, needs four overloads.)
inline std::string larger(const std::string& a, const std::string& b) { return a < b ? b : a; }

inline std::vector<std::function<int(int)>> make_adders() {
    std::vector<std::function<int(int)>> out;
    for (int i = 1; i <= 3; ++i) {
        int step = i * 10;
        out.push_back([step](int x) { return x + step; });
    }
    return out;
}

}  // namespace idioms
