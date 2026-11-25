#include "invalidation.hpp"

#include <map>
#include <string>
#include <vector>

#include "testing.hpp"

using namespace idioms;

TEST(remove_cancelled_keeps_order) {
    std::vector<Order> v{{1, false}, {2, true}, {3, true}, {4, false}, {5, true}, {6, false}};
    remove_cancelled(v);
    CHECK_EQ(v.size(), std::size_t{3});
    CHECK_EQ(v[0].id, 1);
    CHECK_EQ(v[1].id, 4);
    CHECK_EQ(v[2].id, 6);
    std::vector<Order> all{{1, true}, {2, true}};
    remove_cancelled(all);
    CHECK(all.empty());
}

TEST(remove_cancelled_large_is_linear) {
    std::vector<Order> v;
    for (int i = 0; i < 400000; ++i) v.push_back({i, i % 2 == 0});
    remove_cancelled(v);  // an O(n^2) erase loop takes minutes here
    CHECK_EQ(v.size(), std::size_t{200000});
    CHECK_EQ(v.back().id, 399999);
}

TEST(duplicate_evens_in_place) {
    std::vector<int> v{1, 2, 3, 4};
    v.shrink_to_fit();  // the first insert must reallocate
    duplicate_evens(v);
    CHECK(v == (std::vector<int>{1, 2, 2, 3, 4, 4}));
    std::vector<int> evens{2, 4, 6, 8, 10, 12, 14, 16};
    evens.shrink_to_fit();
    duplicate_evens(evens);
    CHECK_EQ(evens.size(), std::size_t{16});
    for (std::size_t i = 0; i < evens.size(); i += 2) CHECK_EQ(evens[i], evens[i + 1]);
    std::vector<int> none{1, 3};
    duplicate_evens(none);
    CHECK(none == (std::vector<int>{1, 3}));
    std::vector<int> empty;
    duplicate_evens(empty);
    CHECK(empty.empty());
}

TEST(expire_map) {
    std::map<int, Quote> book{{1, {10.0, 5}}, {2, {10.5, 15}}, {3, {11.0, 10}}, {4, {9.0, 1}}, {5, {9.5, 20}}};
    CHECK_EQ(expire(book, 10), std::size_t{3});
    CHECK_EQ(book.size(), std::size_t{2});
    CHECK(book.count(2) == 1 && book.count(5) == 1);
    CHECK_EQ(expire(book, 0), std::size_t{0});
    CHECK_EQ(expire(book, 100), std::size_t{2});
    CHECK(book.empty());
}

TEST(append_self_doubles) {
    std::vector<std::string> v{"a long string that is not in SSO", "b", "c"};
    v.shrink_to_fit();
    append_self(v);
    CHECK_EQ(v.size(), std::size_t{6});
    CHECK_EQ(v[3], std::string("a long string that is not in SSO"));
    CHECK_EQ(v[5], std::string("c"));
    std::vector<std::string> e;
    append_self(e);
    CHECK(e.empty());
}

TEST(best_then_grow_no_dangling) {
    std::vector<int> v{5, 9, 3};
    v.shrink_to_fit();
    CHECK_EQ(best_then_grow(v, {1, 2, 4}), 9);
    CHECK_EQ(v.size(), std::size_t{6});
    std::vector<int> w{5};
    w.shrink_to_fit();
    CHECK_EQ(best_then_grow(w, {7, 6}), 7);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
