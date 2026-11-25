#include "binary_search.hpp"

#include <string>
#include <vector>

#include "alloc_counter.hpp"
#include "testing.hpp"

using namespace idioms;

namespace {
std::vector<Level> ladder() { return {{99.5, 10}, {100.0, 20}, {100.5, 5}, {101.0, 7}, {102.5, 1}}; }
}  // namespace

TEST(find_level_values) {
    auto l = ladder();
    CHECK(find_level(l, 100.5) == &l[2]);
    CHECK(find_level(l, 99.5) == &l[0]);
    CHECK(find_level(l, 102.5) == &l[4]);
    CHECK(find_level(l, 100.25) == nullptr);
    CHECK(find_level(l, 1.0) == nullptr);
    CHECK(find_level(l, 200.0) == nullptr);
    CHECK(find_level({}, 1.0) == nullptr);
}

TEST(qty_between_values) {
    auto l = ladder();
    CHECK_EQ(qty_between(l, 100.0, 101.0), 32L);  // inclusive both ends
    CHECK_EQ(qty_between(l, 100.1, 100.9), 5L);
    CHECK_EQ(qty_between(l, 0.0, 1000.0), 43L);
    CHECK_EQ(qty_between(l, 103.0, 104.0), 0L);
    CHECK_EQ(qty_between(l, 101.0, 100.0), 0L);  // empty interval
}

TEST(count_of_values) {
    std::vector<int> v{1, 2, 2, 2, 5, 5, 9};
    CHECK_EQ(count_of(v, 2), std::size_t{3});
    CHECK_EQ(count_of(v, 5), std::size_t{2});
    CHECK_EQ(count_of(v, 3), std::size_t{0});
    CHECK_EQ(count_of(v, 10), std::size_t{0});
}

TEST(insert_sorted_stable) {
    std::vector<int> v{1, 3, 3, 7};
    CHECK_EQ(insert_sorted(v, 3), std::size_t{3});  // after existing 3s
    CHECK_EQ(insert_sorted(v, 0), std::size_t{0});
    CHECK_EQ(insert_sorted(v, 9), std::size_t{6});
    CHECK(v == (std::vector<int>{0, 1, 3, 3, 3, 7, 9}));
    std::vector<int> e;
    CHECK_EQ(insert_sorted(e, 4), std::size_t{0});
}

TEST(first_unfilled_values) {
    CHECK_EQ(first_unfilled({{1, true}, {2, true}, {3, false}, {4, false}}), std::size_t{2});
    CHECK_EQ(first_unfilled({{1, false}}), std::size_t{0});
    CHECK_EQ(first_unfilled({{1, true}}), std::size_t{1});
    CHECK_EQ(first_unfilled({}), std::size_t{0});
}

TEST(symbol_table_lookup_does_not_allocate) {
    SymbolTable t;
    t.add("AAPL", 1);
    t.add("A_VERY_LONG_SYMBOL_NAME_THAT_DOES_NOT_FIT_SSO", 2);
    t.add("MSFT", 3);
    std::string_view long_key = "A_VERY_LONG_SYMBOL_NAME_THAT_DOES_NOT_FIT_SSO";
    long before = alloc_counter::count();
    auto a = t.find("AAPL");
    auto b = t.find(long_key);
    auto c = t.find("A_VERY_LONG_SYMBOL_NAME_THAT_IS_NOT_IN_THE_TABLE_AT_ALL");
    CHECK_EQ(alloc_counter::count(), before);
    CHECK(a.has_value() && *a == 1);
    CHECK(b.has_value() && *b == 2);
    CHECK_FALSE(c.has_value());
    t.add("AAPL", 10);  // overwrite
    CHECK_EQ(*t.find("AAPL"), 10);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
