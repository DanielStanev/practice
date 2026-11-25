// Problem 03: standard iterator adaptors.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <algorithm>
#include <deque>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace idioms {

inline std::vector<int> parse_ints(std::string_view text) {
    (void)text;
    throw std::logic_error("TODO: parse_ints");
}

inline std::string join(const std::vector<int>& values, const char* sep) {
    (void)values;
    (void)sep;
    throw std::logic_error("TODO: join");
}

inline bool erase_last(std::vector<int>& v, int value) {
    (void)v;
    (void)value;
    throw std::logic_error("TODO: erase_last");
}

inline void prepend_all(std::deque<int>& dq, const std::vector<int>& values) {
    (void)dq;
    (void)values;
    throw std::logic_error("TODO: prepend_all");
}

template <class T>
void move_append(std::vector<T>& dst, std::vector<T>& src) {
    (void)dst;
    (void)src;
    throw std::logic_error("TODO: move_append");
}

}  // namespace idioms
