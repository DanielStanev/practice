// Reference solution for problem 22.
#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace idioms {

template <class T, class... Args>
std::unique_ptr<T> make_box(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T, class... Args>
std::size_t emplace_twice(std::vector<T>& vec, Args&&... args) {
    vec.emplace_back(args...);                       // lvalues: copies
    vec.emplace_back(std::forward<Args>(args)...);   // last use: forward
    return vec.size();
}

template <class F, class... Args>
decltype(auto) counted_call(int& counter, F&& f, Args&&... args) {
    ++counter;
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

namespace quiz {
using T_for_g_s = std::string&;
using T_for_g_cs = const std::string&;
using T_for_g_move_s = std::string;
using T_for_g_ref = std::string&;
using T_for_g_literal = int;
using Param_for_g_s = std::string&;
using Param_for_g_move_s = std::string&&;
}  // namespace quiz

}  // namespace idioms
