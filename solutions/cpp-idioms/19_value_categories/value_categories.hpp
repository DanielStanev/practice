// Reference solution for problem 19.
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
    if constexpr (std::is_lvalue_reference_v<T>) return Category::lvalue;
    else if constexpr (std::is_rvalue_reference_v<T>) return Category::xvalue;
    else return Category::prvalue;
}

#define CATEGORY(...) ::idioms::category_of<decltype((__VA_ARGS__))>()

inline constexpr Category q01 = Category::lvalue;   // i
inline constexpr Category q02 = Category::prvalue;  // i + 1
inline constexpr Category q03 = Category::lvalue;   // ++i returns the object itself
inline constexpr Category q04 = Category::prvalue;  // i++ returns a copy of the old value
inline constexpr Category q05 = Category::lvalue;   // *p
inline constexpr Category q06 = Category::lvalue;   // a named rvalue reference is an lvalue
inline constexpr Category q07 = Category::xvalue;   // std::move(i)
inline constexpr Category q08 = Category::lvalue;   // string literals are arrays with static storage
inline constexpr Category q09 = Category::lvalue;   // returns T&
inline constexpr Category q10 = Category::xvalue;   // returns T&&
inline constexpr Category q11 = Category::prvalue;  // returns T
inline constexpr Category q12 = Category::xvalue;   // member of a prvalue: materialised temporary
inline constexpr Category q13 = Category::xvalue;   // member of an xvalue
inline constexpr Category q14 = Category::lvalue;   // operator= returns std::string&

inline std::string_view which(std::string&) { return "lvalue"; }
inline std::string_view which(const std::string&) { return "const lvalue"; }
inline std::string_view which(std::string&&) { return "rvalue"; }

template <class T>
std::string_view relay(T&& x) {
    return which(std::forward<T>(x));
}

}  // namespace idioms
