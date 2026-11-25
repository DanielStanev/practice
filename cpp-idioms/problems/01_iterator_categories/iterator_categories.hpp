// Problem 01: iterator categories and tag dispatch.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <iterator>
#include <stdexcept>
#include <string_view>

namespace idioms {

namespace detail {
// TODO: put your tag-dispatched helpers here.
}  // namespace detail

// Number of increments needed to get from first to last.
// O(1) for random access iterators. Use tag dispatch.
template <class It>
typename std::iterator_traits<It>::difference_type my_distance(It first, It last) {
    (void)first;
    (void)last;
    throw std::logic_error("TODO: my_distance");
}

// Move `it` by n positions. Use tag dispatch.
// Negative n on an input/forward iterator throws std::invalid_argument.
template <class It, class Distance>
void my_advance(It& it, Distance n) {
    (void)it;
    (void)n;
    throw std::logic_error("TODO: my_advance");
}

// "contiguous", "random_access", "bidirectional", "forward", "input" or "output".
// Use the C++20 iterator concepts and if constexpr.
template <class It>
constexpr std::string_view category_name() {
    throw std::logic_error("TODO: category_name");
    return {};
}

}  // namespace idioms
