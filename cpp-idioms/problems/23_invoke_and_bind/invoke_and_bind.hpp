// Problem 23: invoke, member pointers, bind_front.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace idioms {

// The stub uses std::invoke_result_t only so the tests compile; your version
// must not call std::invoke. Change the return type to decltype(auto).
template <class F, class... Args>
std::invoke_result_t<F, Args...> my_invoke(F&& f, Args&&... args) {
    (void)f;
    ((void)args, ...);
    throw std::logic_error("TODO: my_invoke");
}

template <class F, class... Bound>
auto my_bind_front(F&& f, Bound&&... bound) {
    (void)f;
    ((void)bound, ...);
    return [](auto&&...) -> int { throw std::logic_error("TODO: my_bind_front"); };
}

template <class T, class Proj>
long sum_by(const std::vector<T>& items, Proj proj) {
    (void)items;
    (void)proj;
    throw std::logic_error("TODO: sum_by");
}

}  // namespace idioms
