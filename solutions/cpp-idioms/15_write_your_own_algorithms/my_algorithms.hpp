// Reference solution for problem 15.
#pragma once

#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace idioms {

template <std::forward_iterator It, class Pred>
It my_remove_if(It first, It last, Pred pred) {
    while (first != last && !pred(*first)) ++first;
    if (first == last) return first;
    It dest = first;
    for (++first; first != last; ++first) {
        if (!pred(*first)) {
            *dest = std::move(*first);
            ++dest;
        }
    }
    return dest;
}

template <std::forward_iterator It>
It my_unique(It first, It last) {
    if (first == last) return last;
    It dest = first;
    for (It it = std::next(first); it != last; ++it) {
        if (!(*dest == *it)) {
            ++dest;
            if (dest != it) *dest = std::move(*it);
        }
    }
    return ++dest;
}

template <std::forward_iterator It, class T, class Comp = std::less<>>
It my_lower_bound(It first, It last, const T& value, Comp comp = {}) {
    auto count = std::distance(first, last);
    while (count > 0) {
        auto half = count / 2;
        It mid = std::next(first, half);
        if (comp(*mid, value)) {
            first = ++mid;
            count -= half + 1;
        } else {
            count = half;
        }
    }
    return first;
}

template <std::forward_iterator It>
It my_rotate(It first, It middle, It last) {
    if (first == middle) return last;
    if (middle == last) return first;
    It next = middle;
    It result = last;
    while (first != next) {
        std::iter_swap(first++, next++);
        if (next == last) {
            if (result == last) result = first;  // first time the second block runs out
            next = middle;
        } else if (first == middle) {
            middle = next;
        }
    }
    return result;
}

template <std::input_iterator I1, std::input_iterator I2, class Out, class Comp = std::less<>>
Out my_merge(I1 f1, I1 l1, I2 f2, I2 l2, Out out, Comp comp = {}) {
    while (f1 != l1 && f2 != l2) {
        if (comp(*f2, *f1)) {
            *out = *f2;
            ++f2;
        } else {
            *out = *f1;
            ++f1;
        }
        ++out;
    }
    for (; f1 != l1; ++f1, ++out) *out = *f1;
    for (; f2 != l2; ++f2, ++out) *out = *f2;
    return out;
}

}  // namespace idioms
