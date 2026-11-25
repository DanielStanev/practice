// Reference solution for problem 24.
#pragma once

#include <concepts>
#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace idioms {

struct Named {
    static inline int converting_calls = 0;  // counts uses of the forwarding constructor
    std::string name;

    template <class S>
        requires(!std::is_base_of_v<Named, std::remove_cvref_t<S>>) && std::constructible_from<std::string, S>
    explicit Named(S&& s) : name(std::forward<S>(s)) {
        ++converting_calls;
    }

    Named(const Named&) = default;
    Named(Named&&) = default;
    Named& operator=(const Named&) = default;
    Named& operator=(Named&&) = default;

    // A convenience that makes the hijack below compile silently.
    operator std::string_view() const { return name; }
};

class Logger {
public:
    void log(long long id) { entries_.push_back("id:" + std::to_string(id)); }

    template <class T>
        requires(!std::is_integral_v<std::remove_cvref_t<T>>) && std::convertible_to<T, std::string_view>
    void log(T&& text) {
        entries_.push_back("text:" + std::string(std::string_view(text)));
    }
    const std::vector<std::string>& entries() const { return entries_; }

private:
    std::vector<std::string> entries_;
};

template <class Arg>
std::size_t broadcast(Arg&&) {
    return 0;
}

template <class Arg, class F, class... Rest>
std::size_t broadcast(Arg&& arg, F&& f, Rest&&... rest) {
    if constexpr (sizeof...(Rest) == 0) {
        std::forward<F>(f)(std::forward<Arg>(arg));
        return 1;
    } else {
        f(arg);  // lvalue: later functions still need it
        return 1 + broadcast(std::forward<Arg>(arg), std::forward<Rest>(rest)...);
    }
}

template <class R, class V>
void assign_all(R&& range, const V& value) {
    for (auto&& x : range) x = value;
}

}  // namespace idioms
