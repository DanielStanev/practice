// Problem 24: forwarding pitfalls.
// Read README.md first. Implement everything marked TODO.
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

    // TODO: constrain this so it never hijacks copy/move.
    template <class S>
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
    void log(long long id) {
        (void)id;
        throw std::logic_error("TODO: log(id)");
    }
    // TODO: constrain
    template <class T>
    void log(T&& text) {
        (void)text;
        throw std::logic_error("TODO: log(text)");
    }
    const std::vector<std::string>& entries() const { return entries_; }

private:
    std::vector<std::string> entries_;
};

template <class Arg, class... Fs>
std::size_t broadcast(Arg&& arg, Fs&&... fs) {
    (void)arg;
    ((void)fs, ...);
    throw std::logic_error("TODO: broadcast");
}

template <class R, class V>
void assign_all(R&& range, const V& value) {
    (void)range;
    (void)value;
    throw std::logic_error("TODO: assign_all");
}

}  // namespace idioms
