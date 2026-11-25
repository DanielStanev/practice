// Problem 15: write your own standard algorithms.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace idioms {

template <std::forward_iterator It, class Pred>
It my_remove_if(It first, It last, Pred pred) {
    (void)last;
    (void)pred;
    throw std::logic_error("TODO: my_remove_if");
    return first;
}

template <std::forward_iterator It>
It my_unique(It first, It last) {
    (void)last;
    throw std::logic_error("TODO: my_unique");
    return first;
}

template <std::forward_iterator It, class T, class Comp = std::less<>>
It my_lower_bound(It first, It last, const T& value, Comp comp = {}) {
    (void)last;
    (void)value;
    (void)comp;
    throw std::logic_error("TODO: my_lower_bound");
    return first;
}

template <std::forward_iterator It>
It my_rotate(It first, It middle, It last) {
    (void)middle;
    (void)last;
    throw std::logic_error("TODO: my_rotate");
    return first;
}

template <std::input_iterator I1, std::input_iterator I2, class Out, class Comp = std::less<>>
Out my_merge(I1 f1, I1 l1, I2 f2, I2 l2, Out out, Comp comp = {}) {
    (void)f1;
    (void)l1;
    (void)f2;
    (void)l2;
    (void)comp;
    throw std::logic_error("TODO: my_merge");
    return out;
}

}  // namespace idioms
