// Problem 07: sentinels.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <iterator>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

namespace idioms {

struct NullSentinel {
    // TODO: operator== with const char*
};

struct CStr {
    const char* s;
    const char* begin() const { return s; }
    // TODO: end() returning NullSentinel
};

inline long cstr_length(const char* s) {
    (void)s;
    throw std::logic_error("TODO: cstr_length");
}

template <class Pred>
struct UntilSentinel {
    Pred pred{};
    // TODO: operator== against any iterator
};

// [first, first element satisfying pred)
template <class It, class Pred>
auto until(It first, Pred pred) {
    (void)pred;
    throw std::logic_error("TODO: until");
    return std::ranges::subrange(first, first);
}

inline std::string fix_field(const char* msg, int tag) {
    (void)msg;
    (void)tag;
    throw std::logic_error("TODO: fix_field");
}

inline int sum_until_negative(const std::vector<int>& v) {
    (void)v;
    throw std::logic_error("TODO: sum_until_negative");
}

}  // namespace idioms
