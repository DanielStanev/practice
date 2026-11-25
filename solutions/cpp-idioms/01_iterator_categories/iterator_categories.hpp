// Reference solution for problem 01.
#pragma once

#include <iterator>
#include <stdexcept>
#include <string_view>

namespace idioms {

namespace detail {

template <class It>
typename std::iterator_traits<It>::difference_type distance(It first, It last,
                                                            std::input_iterator_tag) {
    typename std::iterator_traits<It>::difference_type n = 0;
    for (; first != last; ++first) ++n;
    return n;
}

template <class It>
typename std::iterator_traits<It>::difference_type distance(It first, It last,
                                                            std::random_access_iterator_tag) {
    return last - first;
}

template <class It, class D>
void advance(It& it, D n, std::input_iterator_tag) {
    if (n < 0) throw std::invalid_argument("my_advance: negative step on a forward iterator");
    for (; n > 0; --n) ++it;
}

template <class It, class D>
void advance(It& it, D n, std::bidirectional_iterator_tag) {
    for (; n > 0; --n) ++it;
    for (; n < 0; ++n) --it;
}

template <class It, class D>
void advance(It& it, D n, std::random_access_iterator_tag) {
    it += n;
}

}  // namespace detail

template <class It>
typename std::iterator_traits<It>::difference_type my_distance(It first, It last) {
    using Cat = typename std::iterator_traits<It>::iterator_category;
    return detail::distance(first, last, Cat{});
}

template <class It, class Distance>
void my_advance(It& it, Distance n) {
    using Cat = typename std::iterator_traits<It>::iterator_category;
    detail::advance(it, n, Cat{});
}

template <class It>
constexpr std::string_view category_name() {
    if constexpr (std::contiguous_iterator<It>) return "contiguous";
    else if constexpr (std::random_access_iterator<It>) return "random_access";
    else if constexpr (std::bidirectional_iterator<It>) return "bidirectional";
    else if constexpr (std::forward_iterator<It>) return "forward";
    else if constexpr (std::input_iterator<It>) return "input";
    else return "output";
}

}  // namespace idioms
