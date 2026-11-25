#include "projections.hpp"

#include <ranges>
#include <string>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
std::vector<Order> sample() {
    return {{1, 100.0, 30, 5}, {2, 101.0, 20, 7}, {3, 100.0, 10, 7}, {4, 101.0, 20, 1}, {5, 99.5, 5, 3}};
}
[[maybe_unused]] std::vector<int> make_ints() { return {1, 2, 3}; }
}  // namespace

TEST(sort_book_order) {
    auto v = sample();
    sort_book(v);
    std::vector<int> ids;
    for (auto& o : v) ids.push_back(o.id);
    CHECK(ids == (std::vector<int>{2, 4, 3, 1, 5}));
}

TEST(largest_order_first_on_tie) {
    auto v = sample();
    const Order* o = largest_order(v);
    CHECK(o != nullptr);
    CHECK_EQ(o->id, 2);
    CHECK(o == &v[1]);
    CHECK(largest_order({}) == nullptr);
}

TEST(find_by_id_values) {
    auto v = sample();
    CHECK(find_by_id(v, 4) == &v[3]);
    CHECK(find_by_id(v, 9) == nullptr);
}

TEST(qty_extremes_values) {
    auto [lo, hi] = qty_extremes(sample());
    CHECK_EQ(lo, 1L);
    CHECK_EQ(hi, 7L);
}

TEST(dangling_and_borrowed_at_compile_time) {
    // A temporary vector is not borrowed: the result is `dangling`.
    CHECK_TRAIT(std::is_same_v<decltype(std::ranges::find(make_ints(), 2)), std::ranges::dangling>);
    // IntSpan must be borrowed: the result is a usable iterator.
    CHECK_TRAIT(std::ranges::borrowed_range<IntSpan>);
    CHECK_TRAIT(std::is_same_v<decltype(std::ranges::find(IntSpan{}, 2)), const int*>);
}

TEST(index_of_first_negative_values) {
    std::vector<int> v{4, 2, -1, 3, -8};
    CHECK_EQ(index_of_first_negative(IntSpan{v.data(), v.size()}), 2L);
    CHECK_EQ(index_of_first_negative(IntSpan{v.data(), 2}), -1L);
    CHECK_EQ(index_of_first_negative(IntSpan{}), -1L);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
