// Reference solution for problem 03.
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
    std::istringstream in{std::string(text)};
    return {std::istream_iterator<int>(in), std::istream_iterator<int>()};
}

inline std::string join(const std::vector<int>& values, const char* sep) {
    if (values.empty()) return {};
    std::ostringstream os;
    std::copy(values.begin(), std::prev(values.end()), std::ostream_iterator<int>(os, sep));
    os << values.back();
    return os.str();
}

inline bool erase_last(std::vector<int>& v, int value) {
    auto r = std::find(v.rbegin(), v.rend(), value);
    if (r == v.rend()) return false;
    v.erase(std::next(r).base());
    return true;
}

inline void prepend_all(std::deque<int>& dq, const std::vector<int>& values) {
    std::copy(values.rbegin(), values.rend(), std::front_inserter(dq));
}

template <class T>
void move_append(std::vector<T>& dst, std::vector<T>& src) {
    dst.reserve(dst.size() + src.size());
    dst.insert(dst.end(), std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()));
    src.clear();
}

}  // namespace idioms
