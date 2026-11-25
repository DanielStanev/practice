#include "sentinels.hpp"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
template <class R>
std::string to_string_range(R&& r) {
    std::string out;
    for (char c : r) out.push_back(c);
    return out;
}
template <class S>
constexpr bool kCStrReady = std::ranges::range<S> && requires(S s) { std::ranges::find(s, 'x'); };
}  // namespace

TEST(cstr_is_non_common_range) {
    CHECK_TRAIT(std::ranges::range<CStr>);
    CHECK_TRAIT(!std::ranges::common_range<CStr>);
    CHECK_TRAIT(std::sentinel_for<NullSentinel, const char*>);
}

template <class S>
void cstr_algorithms_impl() {
    if constexpr (kCStrReady<S>) {
        S s{"bid,ask,last"};
        auto it = std::ranges::find(s, ',');
        CHECK_EQ(*it, ',');
        CHECK_EQ(it - s.begin(), 3);
        CHECK_EQ(std::ranges::count(s, ','), 2);
        CHECK_EQ(to_string_range(S{"abc"}), std::string("abc"));
        auto upper = S{"xyz"} | std::views::transform([](char c) { return char(c - 32); });
        CHECK_EQ(to_string_range(upper), std::string("XYZ"));
    } else {
        ::testing::fail(__FILE__, __LINE__, "CStr is not a range yet");
    }
}
TEST(cstr_with_ranges_algorithms) { cstr_algorithms_impl<CStr>(); }

TEST(cstr_length_values) {
    CHECK_EQ(cstr_length(""), 0L);
    CHECK_EQ(cstr_length("hello"), 5L);
    CHECK_EQ(cstr_length("a\0b"), 1L);
}

TEST(until_on_pointer) {
    const char* text = "AAPL|100|187.5";
    auto r = until(text, [](char c) { return c == '|'; });
    CHECK_EQ(to_string_range(r), std::string("AAPL"));
    CHECK_EQ(std::ranges::distance(r), 4);
}

TEST(until_on_input_iterator) {
    std::istringstream in("header;body");
    auto r = until(std::istreambuf_iterator<char>(in), [](char c) { return c == ';'; });
    CHECK_EQ(to_string_range(r), std::string("header"));
}

TEST(until_sentinel_is_a_sentinel) {
    auto pred = [](char c) { return c == 'x'; };
    CHECK_TRAIT(std::sentinel_for<UntilSentinel<decltype(pred)>, const char*>);
    CHECK_TRAIT(std::sentinel_for<UntilSentinel<decltype(pred)>, std::string::iterator>);
}

TEST(fix_fields) {
    const char* msg = "8=FIX.4.2\x01" "35=D\x01" "55=AAPL\x01" "54=1\x01" "38=100\x01";
    CHECK_EQ(fix_field(msg, 8), std::string("FIX.4.2"));
    CHECK_EQ(fix_field(msg, 35), std::string("D"));
    CHECK_EQ(fix_field(msg, 55), std::string("AAPL"));
    CHECK_EQ(fix_field(msg, 38), std::string("100"));
    CHECK_EQ(fix_field(msg, 44), std::string(""));
    CHECK_EQ(fix_field("", 8), std::string(""));
    CHECK_EQ(fix_field("10=abc", 10), std::string("abc"));  // no trailing SOH
    CHECK_EQ(fix_field("5=x\x01" "55=y\x01", 5), std::string("x"));  // no prefix confusion
    CHECK_EQ(fix_field("55=y\x01" "5=x\x01", 5), std::string("x"));
}

TEST(sum_with_legacy_accumulate) {
    CHECK_EQ(sum_until_negative({1, 2, 3, -1, 100}), 6);
    CHECK_EQ(sum_until_negative({-5, 1}), 0);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
