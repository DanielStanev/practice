// Problem 21: noexcept, move_if_noexcept, implicit moves on return.
// Read README.md first. Implement everything marked TODO.
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
    // TODO: these are correct but something is missing.
    Message(Message&& o) : topic(std::move(o.topic)), payload(std::move(o.payload)) { ++moves; }
    Message& operator=(Message&& o) {
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
    // TODO: conditional noexcept
    Box(Box&& o) : value(std::move(o.value)) { ++moves; }
    Box& operator=(Box&& o) {
        value = std::move(o.value);
        ++moves;
        return *this;
    }
};

template <class T>
void relocate(T* src, T* dst, std::size_t n) {
    (void)src;
    (void)dst;
    (void)n;
    throw std::logic_error("TODO: relocate");
}

template <class T>
T take_last(std::vector<T> v) {
    throw std::logic_error("TODO: take_last");
    return v.back();
}

template <class A, class B>
A name_of(std::pair<A, B> p) {
    throw std::logic_error("TODO: name_of");
    return p.first;
}

}  // namespace idioms
