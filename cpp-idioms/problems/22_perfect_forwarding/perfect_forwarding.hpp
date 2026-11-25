// Problem 22: perfect forwarding.
// Read README.md first. Implement everything marked TODO.
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
    ((void)args, ...);
    throw std::logic_error("TODO: make_box");
}

template <class T, class... Args>
std::size_t emplace_twice(std::vector<T>& vec, Args&&... args) {
    (void)vec;
    ((void)args, ...);
    throw std::logic_error("TODO: emplace_twice");
}

// The stub spells out the return type so the tests compile. Replace it with
// decltype(auto) (and think about why both give the same type here).
template <class F, class... Args>
std::invoke_result_t<F, Args...> counted_call(int& counter, F&& f, Args&&... args) {
    (void)counter;
    (void)f;
    ((void)args, ...);
    throw std::logic_error("TODO: counted_call");
}

// Quiz. For `template <class T> void g(T&& x);` and
//   std::string s; const std::string cs; std::string& ref = s;
// replace `void` with what T deduces to in each call.
namespace quiz {
using T_for_g_s = void;                     // g(s)
using T_for_g_cs = void;                    // g(cs)
using T_for_g_move_s = void;                // g(std::move(s))
using T_for_g_ref = void;                   // g(ref)
using T_for_g_literal = void;               // g(42)
using Param_for_g_s = void;                 // the type of x in g(s), after collapsing
using Param_for_g_move_s = void;            // the type of x in g(std::move(s))
}  // namespace quiz

}  // namespace idioms
