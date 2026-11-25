// Reference solution for problem 02.
#pragma once

#include <cstddef>
#include <iterator>
#include <stdexcept>

namespace idioms {

template <std::forward_iterator It>
It middle(It first, It last) {
    It slow = first;
    It fast = first;
    while (fast != last) {
        ++fast;
        if (fast == last) break;
        ++fast;
        ++slow;
    }
    return slow;
}

template <std::random_access_iterator It>
It middle(It first, It last) {
    return first + (last - first) / 2;
}

template <std::forward_iterator It>
It last_n(It first, It last, std::ptrdiff_t n) {
    It lead = first;
    for (std::ptrdiff_t i = 0; i < n; ++i) {
        if (lead == last) return first;
        ++lead;
    }
    It lag = first;
    while (lead != last) {
        ++lead;
        ++lag;
    }
    return lag;
}

template <std::bidirectional_iterator It>
It last_n(It first, It last, std::ptrdiff_t n) {
    for (std::ptrdiff_t i = 0; i < n && last != first; ++i) --last;
    return last;
}

template <std::random_access_iterator It>
It last_n(It first, It last, std::ptrdiff_t n) {
    return (last - first) <= n ? first : last - n;
}

template <std::bidirectional_iterator It>
bool is_palindrome(It first, It last) {
    while (first != last) {
        --last;
        if (first == last) break;
        if (!(*first == *last)) return false;
        ++first;
    }
    return true;
}

}  // namespace idioms
