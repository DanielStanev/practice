#include "my_algorithms.hpp"

#include <algorithm>
#include <forward_list>
#include <iterator>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
template <class C>
std::vector<int> to_vec(const C& c) {
    return std::vector<int>(c.begin(), c.end());
}
}  // namespace

TEST(remove_if_forward_list) {
    std::forward_list<int> l{1, 2, 3, 4, 5, 6, 7};
    auto e = my_remove_if(l.begin(), l.end(), [](int x) { return x % 3 == 0; });
    CHECK_EQ(std::distance(l.begin(), e), 5L);
    CHECK((std::vector<int>(l.begin(), e)) == (std::vector<int>{1, 2, 4, 5, 7}));
}

TEST(remove_if_move_only) {
    std::vector<std::unique_ptr<int>> v;
    for (int i = 0; i < 10; ++i) v.push_back(std::make_unique<int>(i));
    auto e = my_remove_if(v.begin(), v.end(), [](const auto& p) { return *p % 2 == 0; });
    v.erase(e, v.end());
    CHECK_EQ(v.size(), std::size_t{5});
    for (std::size_t i = 0; i < v.size(); ++i) CHECK_EQ(*v[i], int(2 * i + 1));
}

TEST(remove_if_edges) {
    std::vector<int> none{1, 3};
    CHECK(my_remove_if(none.begin(), none.end(), [](int) { return false; }) == none.end());
    std::vector<int> all{1, 3};
    CHECK(my_remove_if(all.begin(), all.end(), [](int) { return true; }) == all.begin());
    std::vector<int> empty;
    CHECK(my_remove_if(empty.begin(), empty.end(), [](int) { return true; }) == empty.end());
}

TEST(unique_values) {
    std::forward_list<int> l{1, 1, 2, 2, 2, 3, 1, 1};
    auto e = my_unique(l.begin(), l.end());
    CHECK((std::vector<int>(l.begin(), e)) == (std::vector<int>{1, 2, 3, 1}));
    std::vector<std::string> s{"a", "a", "b"};
    s.erase(my_unique(s.begin(), s.end()), s.end());
    CHECK(s == (std::vector<std::string>{"a", "b"}));
    std::vector<int> empty;
    CHECK(my_unique(empty.begin(), empty.end()) == empty.end());
}

TEST(unique_no_self_move) {
    // Moving a string into itself may empty it; the kept prefix must be intact.
    std::vector<std::string> s{"alpha", "bravo", "charlie", "charlie", "delta"};
    s.erase(my_unique(s.begin(), s.end()), s.end());
    CHECK(s == (std::vector<std::string>{"alpha", "bravo", "charlie", "delta"}));
}

TEST(lower_bound_matches_std) {
    std::vector<int> v{1, 3, 3, 5, 8, 13, 21};
    for (int x = 0; x <= 22; ++x)
        CHECK(my_lower_bound(v.begin(), v.end(), x) == std::lower_bound(v.begin(), v.end(), x));
    std::forward_list<int> l(v.begin(), v.end());
    CHECK_EQ(*my_lower_bound(l.begin(), l.end(), 4), 5);
    CHECK(my_lower_bound(l.begin(), l.end(), 100) == l.end());
}

TEST(lower_bound_log_comparisons) {
    std::vector<int> v(1 << 16);
    for (int i = 0; i < (1 << 16); ++i) v[i] = 2 * i;
    int comparisons = 0;
    auto it = my_lower_bound(v.begin(), v.end(), 7777, [&](int a, int b) {
        ++comparisons;
        return a < b;
    });
    CHECK_EQ(*it, 7778);
    CHECK(comparisons <= 17);
}

TEST(rotate_all_splits) {
    for (int n = 0; n <= 9; ++n) {
        for (int m = 0; m <= n; ++m) {
            std::forward_list<int> l;
            for (int i = n - 1; i >= 0; --i) l.push_front(i);
            std::vector<int> ref(n);
            for (int i = 0; i < n; ++i) ref[i] = i;
            auto mid = std::next(l.begin(), m);
            auto r = my_rotate(l.begin(), mid, l.end());
            auto ref_r = std::rotate(ref.begin(), ref.begin() + m, ref.end());
            CHECK(to_vec(l) == ref);
            CHECK_EQ(std::distance(l.begin(), r), ref_r - ref.begin());
        }
    }
}

TEST(merge_stable) {
    using P = std::pair<int, char>;
    std::vector<P> a{{1, 'a'}, {3, 'a'}, {3, 'a'}, {7, 'a'}};
    std::vector<P> b{{2, 'b'}, {3, 'b'}, {8, 'b'}};
    std::vector<P> out;
    auto byKey = [](const P& x, const P& y) { return x.first < y.first; };
    my_merge(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(out), byKey);
    std::vector<P> expect{{1, 'a'}, {2, 'b'}, {3, 'a'}, {3, 'a'}, {3, 'b'}, {7, 'a'}, {8, 'b'}};
    CHECK(out == expect);
}

TEST(merge_input_iterators) {
    std::istringstream s1("1 4 9"), s2("2 3 10 11");
    std::vector<int> out(7);
    auto e = my_merge(std::istream_iterator<int>(s1), std::istream_iterator<int>(), std::istream_iterator<int>(s2),
                      std::istream_iterator<int>(), out.begin());
    CHECK(e == out.end());
    CHECK(out == (std::vector<int>{1, 2, 3, 4, 9, 10, 11}));
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
