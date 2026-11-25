// Reference solution for problem 07.
#pragma once

#include <iterator>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

namespace idioms {

struct NullSentinel {
    friend bool operator==(const char* p, NullSentinel) { return *p == '\0'; }
};

struct CStr {
    const char* s;
    const char* begin() const { return s; }
    NullSentinel end() const { return {}; }
};

inline long cstr_length(const char* s) { return static_cast<long>(std::ranges::distance(CStr{s})); }

template <class Pred>
struct UntilSentinel {
    Pred pred{};
    template <class I>
    friend bool operator==(const I& it, const UntilSentinel& s) {
        return s.pred(*it);
    }
};

template <class It, class Pred>
auto until(It first, Pred pred) {
    return std::ranges::subrange(first, UntilSentinel<Pred>{pred});
}

inline std::string fix_field(const char* msg, int tag) {
    constexpr auto is_eq = [](char c) { return c == '='; };
    constexpr auto is_soh = [](char c) { return c == '\x01'; };
    constexpr auto is_end = [](char c) { return c == '\x01' || c == '\0'; };
    const char* p = msg;
    while (*p != '\0') {
        auto tag_range = until(p, is_eq);
        std::string tag_text(tag_range.begin(), std::ranges::next(tag_range.begin(), tag_range.end()));
        p += tag_text.size() + 1;  // skip "tag="
        auto value_range = until(p, is_end);
        const char* value_end = std::ranges::next(value_range.begin(), value_range.end());
        if (std::stoi(tag_text) == tag) return std::string(p, value_end);
        p = is_soh(*value_end) ? value_end + 1 : value_end;
    }
    return {};
}

inline int sum_until_negative(const std::vector<int>& v) {
    auto r = until(v.begin(), [](int x) { return x < 0; }) | std::views::common;
    return std::accumulate(r.begin(), r.end(), 0);
}

}  // namespace idioms
