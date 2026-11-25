// Reference solution for problem 21.
#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace idioms {

struct Message {
    static inline int moves = 0;
    std::string topic;
    std::vector<int> payload;

    Message(std::string t, std::vector<int> p) : topic(std::move(t)), payload(std::move(p)) {}
    Message(const Message&) = default;
    Message& operator=(const Message&) = default;
    Message(Message&& o) noexcept : topic(std::move(o.topic)), payload(std::move(o.payload)) { ++moves; }
    Message& operator=(Message&& o) noexcept {
        topic = std::move(o.topic);
        payload = std::move(o.payload);
        ++moves;
        return *this;
    }
};

template <class T>
struct Box {
    static inline int moves = 0;
    T value;

    explicit Box(T v) : value(std::move(v)) {}
    Box(const Box&) = default;
    Box& operator=(const Box&) = default;
    Box(Box&& o) noexcept(std::is_nothrow_move_constructible_v<T>) : value(std::move(o.value)) { ++moves; }
    Box& operator=(Box&& o) noexcept(std::is_nothrow_move_assignable_v<T>) {
        value = std::move(o.value);
        ++moves;
        return *this;
    }
};

template <class T>
void relocate(T* src, T* dst, std::size_t n) {
    std::size_t i = 0;
    try {
        for (; i < n; ++i) std::construct_at(dst + i, std::move_if_noexcept(src[i]));
    } catch (...) {
        std::destroy_n(dst, i);
        throw;
    }
    std::destroy_n(src, n);
}

template <class T>
T take_last(std::vector<T> v) {
    return std::move(v.back());
}

template <class A, class B>
A name_of(std::pair<A, B> p) {
    return std::move(p.first);
}

}  // namespace idioms
