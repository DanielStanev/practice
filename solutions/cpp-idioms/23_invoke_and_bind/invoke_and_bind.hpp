// Reference solution for problem 23.
#pragma once

#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace idioms {

namespace detail {
template <class T>
struct is_reference_wrapper : std::false_type {};
template <class U>
struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};

template <class M, class C>
C class_of(M C::*);

// Turns the "object" argument into an expression of the class type,
// keeping its value category when it is the object itself.
template <class Class, class Obj>
decltype(auto) get_object(Obj&& obj) {
    if constexpr (std::is_base_of_v<Class, std::decay_t<Obj>>) {
        return std::forward<Obj>(obj);
    } else if constexpr (is_reference_wrapper<std::decay_t<Obj>>::value) {
        return obj.get();
    } else {
        return *std::forward<Obj>(obj);
    }
}
}  // namespace detail

template <class F, class... Args>
decltype(auto) my_invoke(F&& f, Args&&... args) {
    using Fn = std::decay_t<F>;
    if constexpr (std::is_member_function_pointer_v<Fn>) {
        return [&](auto&& obj, auto&&... rest) -> decltype(auto) {
            using Class = decltype(detail::class_of(f));
            return (detail::get_object<Class>(std::forward<decltype(obj)>(obj)).*f)(
                std::forward<decltype(rest)>(rest)...);
        }(std::forward<Args>(args)...);
    } else if constexpr (std::is_member_object_pointer_v<Fn>) {
        static_assert(sizeof...(Args) == 1, "a data member pointer takes exactly one object");
        using Class = decltype(detail::class_of(f));
        return [&](auto&& obj) -> decltype(auto) {
            return (detail::get_object<Class>(std::forward<decltype(obj)>(obj)).*f);
        }(std::forward<Args>(args)...);
    } else {
        return std::forward<F>(f)(std::forward<Args>(args)...);
    }
}

template <class F, class... Bound>
auto my_bind_front(F&& f, Bound&&... bound) {
    return [f = std::forward<F>(f), ... b = std::forward<Bound>(bound)](auto&&... rest) mutable -> decltype(auto) {
        return my_invoke(f, b..., std::forward<decltype(rest)>(rest)...);
    };
}

template <class T, class Proj>
long sum_by(const std::vector<T>& items, Proj proj) {
    long total = 0;
    for (const T& item : items) total += my_invoke(proj, item);
    return total;
}

}  // namespace idioms
