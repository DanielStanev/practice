// Problem 19: value categories.
// Read README.md first. Implement category_of and which/relay, then answer the quiz.
#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace idioms {

enum class Category { unknown, lvalue, xvalue, prvalue };

template <class T>
constexpr Category category_of() {
    throw std::logic_error("TODO: category_of");
    return Category::unknown;
}

#define CATEGORY(...) ::idioms::category_of<decltype((__VA_ARGS__))>()

// ---- Quiz -------------------------------------------------------------------
// Given these declarations (they are repeated in the test file):
//
//   int i = 0;            int* p = &i;          int&& rr = 5;
//   std::string s;        struct S { int m; };  S obj;
//   int& get_ref();       int&& get_xref();     int get_val();
//   S make_s();
//
// what is the category of each expression?
inline constexpr Category q01 = Category::unknown;  // i
inline constexpr Category q02 = Category::unknown;  // i + 1
inline constexpr Category q03 = Category::unknown;  // ++i
inline constexpr Category q04 = Category::unknown;  // i++
inline constexpr Category q05 = Category::unknown;  // *p
inline constexpr Category q06 = Category::unknown;  // rr
inline constexpr Category q07 = Category::unknown;  // std::move(i)
inline constexpr Category q08 = Category::unknown;  // "hello"
inline constexpr Category q09 = Category::unknown;  // get_ref()
inline constexpr Category q10 = Category::unknown;  // get_xref()
inline constexpr Category q11 = Category::unknown;  // get_val()
inline constexpr Category q12 = Category::unknown;  // make_s().m
inline constexpr Category q13 = Category::unknown;  // std::move(obj).m
inline constexpr Category q14 = Category::unknown;  // s = "x"   (the assignment expression)

// ---- Overloads --------------------------------------------------------------
inline std::string_view which(std::string&) { throw std::logic_error("TODO: which(std::string&)"); }
inline std::string_view which(const std::string&) { throw std::logic_error("TODO: which(const std::string&)"); }
inline std::string_view which(std::string&&) { throw std::logic_error("TODO: which(std::string&&)"); }

template <class T>
std::string_view relay(T&& x) {
    (void)x;
    throw std::logic_error("TODO: relay");
}

}  // namespace idioms
