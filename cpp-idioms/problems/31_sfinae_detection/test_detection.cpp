#include "detection.hpp"

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "alloc_counter.hpp"
#include "testing.hpp"

using namespace idioms;

namespace {
struct Point {
    int x, y;
};
std::ostream& operator<<(std::ostream& os, const Point& p) { return os << "(" << p.x << "," << p.y << ")"; }
struct Opaque {};
struct WeirdSize {
    int size() const { return 3; }
};
}  // namespace

TEST(detect_size) {
    CHECK_TRAIT(has_size_v<std::vector<int>>);
    CHECK_TRAIT(has_size_v<std::string>);
    CHECK_TRAIT(has_size_v<WeirdSize>);
    CHECK_TRAIT(!has_size_v<std::forward_list<int>>);
    CHECK_TRAIT(!has_size_v<int[4]>);
    CHECK_TRAIT(!has_size_v<int>);
}

TEST(detect_reserve) {
    CHECK_TRAIT(has_reserve_v<std::vector<int>>);
    CHECK_TRAIT(has_reserve_v<std::string>);
    CHECK_TRAIT(has_reserve_v<std::unordered_map<int, int>>);
    CHECK_TRAIT(!has_reserve_v<std::list<int>>);
    CHECK_TRAIT(!has_reserve_v<std::deque<int>>);
    CHECK_TRAIT(!has_reserve_v<const std::vector<int>>);  // reserve is non-const
}

TEST(detect_mapped_type) {
    CHECK_TRAIT(has_mapped_type_v<std::map<int, int>>);
    CHECK_TRAIT(has_mapped_type_v<std::unordered_map<std::string, double>>);
    CHECK_TRAIT(!has_mapped_type_v<std::set<int>>);
    CHECK_TRAIT(!has_mapped_type_v<int>);
}

TEST(detect_streamable) {
    CHECK_TRAIT(is_streamable_v<int>);
    CHECK_TRAIT(is_streamable_v<std::string>);
    CHECK_TRAIT(is_streamable_v<Point>);
    CHECK_TRAIT(!is_streamable_v<Opaque>);
    CHECK_TRAIT(!is_streamable_v<std::vector<int>>);
}

TEST(container_size_both_paths) {
    std::forward_list<int> fl{1, 2, 3, 4};
    int arr[7] = {};
    CHECK_EQ(container_size(std::vector<int>(5)), std::size_t{5});
    CHECK_EQ(container_size(fl), std::size_t{4});
    CHECK_EQ(container_size(arr), std::size_t{7});
    CHECK_EQ(container_size(WeirdSize{}), std::size_t{3});
}

TEST(to_text_all_branches) {
    CHECK_EQ(to_text(std::string("abc")), std::string("abc"));
    CHECK_EQ(to_text(42), std::string("42"));
    CHECK_EQ(to_text(Point{1, 2}), std::string("(1,2)"));
    CHECK_EQ(to_text(Opaque{}), std::string("<?>"));
    CHECK_EQ(to_text("lit"), std::string("lit"));  // const char[4] is streamable
}

TEST(append_all_reserves_when_possible) {
    std::vector<int> dst{1};
    dst.shrink_to_fit();
    std::forward_list<int> src{2, 3, 4, 5, 6, 7, 8, 9};
    long before = alloc_counter::count();
    append_all(dst, src);
    CHECK_EQ(alloc_counter::count(), before + 1);  // exactly one reallocation thanks to reserve
    CHECK(dst == (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}));
    std::set<int> s{1};
    append_all(s, std::vector<int>{3, 2});  // no reserve, still works
    CHECK(s == (std::set<int>{1, 2, 3}));
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
