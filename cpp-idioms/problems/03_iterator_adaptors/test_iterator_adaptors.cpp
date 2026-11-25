#include "iterator_adaptors.hpp"

#include <deque>
#include <string>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
struct Counted {
    static inline int copies = 0;
    std::string s;
    explicit Counted(std::string v) : s(std::move(v)) {}
    Counted(const Counted& o) : s(o.s) { ++copies; }
    Counted(Counted&& o) noexcept : s(std::move(o.s)) {}
    Counted& operator=(const Counted& o) { s = o.s; ++copies; return *this; }
    Counted& operator=(Counted&& o) noexcept { s = std::move(o.s); return *this; }
};
}  // namespace

TEST(parse_ints_basic) {
    CHECK(parse_ints("3 1 4 1 5") == (std::vector<int>{3, 1, 4, 1, 5}));
    CHECK(parse_ints("  -7\n\t42  ") == (std::vector<int>{-7, 42}));
    CHECK(parse_ints("").empty());
}

TEST(parse_ints_stops_at_garbage) {
    CHECK(parse_ints("1 2 x 3") == (std::vector<int>{1, 2}));
    CHECK(parse_ints("abc 1").empty());
}

TEST(join_basic) {
    CHECK_EQ(join({1, 2, 3}, ", "), std::string("1, 2, 3"));
    CHECK_EQ(join({42}, ", "), std::string("42"));
    CHECK_EQ(join({}, ", "), std::string(""));
    CHECK_EQ(join({-1, 0}, "|"), std::string("-1|0"));
}

TEST(erase_last_occurrence) {
    std::vector<int> v{1, 2, 3, 2, 4};
    CHECK(erase_last(v, 2));
    CHECK(v == (std::vector<int>{1, 2, 3, 4}));
    CHECK(erase_last(v, 4));  // last element: the classic off-by-one trap
    CHECK(v == (std::vector<int>{1, 2, 3}));
    CHECK(erase_last(v, 1));  // first element
    CHECK(v == (std::vector<int>{2, 3}));
    CHECK_FALSE(erase_last(v, 99));
    CHECK(v == (std::vector<int>{2, 3}));
}

TEST(prepend_keeps_order) {
    std::deque<int> dq{9};
    prepend_all(dq, {1, 2, 3});
    CHECK(dq == (std::deque<int>{1, 2, 3, 9}));
    prepend_all(dq, {});
    CHECK(dq == (std::deque<int>{1, 2, 3, 9}));
}

TEST(move_append_no_copies) {
    std::vector<Counted> dst, src;
    dst.emplace_back("a");
    for (int i = 0; i < 50; ++i) src.emplace_back(std::string(40, char('a' + i % 26)));
    Counted::copies = 0;
    move_append(dst, src);
    CHECK_EQ(Counted::copies, 0);
    CHECK_EQ(dst.size(), std::size_t{51});
    CHECK(src.empty());
    CHECK_EQ(dst[1].s, std::string(40, 'a'));
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
