// Problem 02: overloading on iterator concepts.
// Read README.md first. Replace each TODO overload set with constrained overloads.
#pragma once

#include <cstddef>
#include <iterator>
#include <stdexcept>

namespace idioms {

// TODO: provide a std::forward_iterator overload (tortoise and hare) and a
// std::random_access_iterator overload (O(1)).
template <std::forward_iterator It>
It middle(It first, It last) {
    (void)last;
    throw std::logic_error("TODO: middle");
    return first;
}

// TODO: forward (two pointers), bidirectional (walk back) and random access (O(1)).
template <std::forward_iterator It>
It last_n(It first, It last, std::ptrdiff_t n) {
    (void)last;
    (void)n;
    throw std::logic_error("TODO: last_n");
    return first;
}

// TODO: constrain to std::bidirectional_iterator.
template <class It>
bool is_palindrome(It first, It last) {
    (void)first;
    (void)last;
    throw std::logic_error("TODO: is_palindrome");
}

}  // namespace idioms
