#include "forward_iterator.hpp"

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

#include "testing.hpp"

using idioms::SList;
using idioms::StepRange;

namespace {

// These helpers only instantiate their bodies when the concept holds, so an
// unfinished iterator fails a test instead of breaking the build.
template <class R>
std::vector<long> collect(const R& r) {
    if constexpr (std::forward_iterator<decltype(r.begin())>) {
        std::vector<long> out;
        for (auto it = r.begin(); it != r.end(); it++) out.push_back(*it);
        return out;
    } else {
        ::testing::fail(__FILE__, __LINE__, "iterator is not a std::forward_iterator yet");
    }
}

template <class It, class V>
concept ValueIs = requires { requires std::is_same_v<std::iter_value_t<It>, V>; };
template <class It, class Ref>
concept ReferenceIs = requires { requires std::is_same_v<std::iter_reference_t<It>, Ref>; };

template <class L>
constexpr bool kListReady = std::forward_iterator<typename L::iterator> &&
                            std::forward_iterator<typename L::const_iterator>;

template <class L>
void require_ready() {
    if (!kListReady<L>) ::testing::fail(__FILE__, __LINE__, "SList iterators are not std::forward_iterator yet");
}

}  // namespace

TEST(step_range_is_forward_iterator) {
    CHECK_TRAIT(std::forward_iterator<StepRange::iterator>);
    CHECK_TRAIT(ValueIs<StepRange::iterator, long>);
}

TEST(step_range_values) {
    CHECK(collect(StepRange(0, 10, 3)) == (std::vector<long>{0, 3, 6, 9}));
    CHECK(collect(StepRange(0, 9, 3)) == (std::vector<long>{0, 3, 6}));
    CHECK(collect(StepRange(-4, 1, 2)) == (std::vector<long>{-4, -2, 0}));
    CHECK(collect(StepRange(1, 2, 100)) == (std::vector<long>{1}));
    CHECK(collect(StepRange(5, 5, 1)).empty());
    CHECK(collect(StepRange(7, 3, 1)).empty());
}

template <class R>
void step_range_with_algorithms_impl(const R& r) {
    if constexpr (std::forward_iterator<decltype(r.begin())>) {
        CHECK_EQ(std::count_if(r.begin(), r.end(), [](long v) { return v % 2 == 0; }), 50);
        auto it = std::find(r.begin(), r.end(), 42L);
        CHECK(it != r.end());
        CHECK_EQ(*it, 42L);
        CHECK_EQ(std::ranges::distance(r.begin(), r.end()), 100);
        CHECK(std::ranges::find(r.begin(), r.end(), 1000L) == r.end());
        // multi-pass: two copies advance independently
        auto a = r.begin();
        auto b = a;
        ++a;
        CHECK_EQ(*b, 0L);
        CHECK_EQ(*a, 1L);
    } else {
        ::testing::fail(__FILE__, __LINE__, "not a forward iterator yet");
    }
}

TEST(step_range_with_algorithms) { step_range_with_algorithms_impl(StepRange(0, 100, 1)); }

TEST(slist_iterators_are_forward) {
    CHECK_TRAIT(std::forward_iterator<SList<int>::iterator>);
    CHECK_TRAIT(std::forward_iterator<SList<int>::const_iterator>);
    CHECK_TRAIT(ReferenceIs<SList<int>::iterator, int&>);
    CHECK_TRAIT(ReferenceIs<SList<int>::const_iterator, const int&>);
}

TEST(slist_const_conversion_one_way) {
    CHECK_TRAIT(std::is_convertible_v<SList<int>::iterator, SList<int>::const_iterator>);
    CHECK_TRAIT(!std::is_convertible_v<SList<int>::const_iterator, SList<int>::iterator>);
    CHECK_TRAIT(!std::is_constructible_v<SList<int>::iterator, SList<int>::const_iterator>);
    CHECK_TRAIT(!std::is_copy_constructible_v<SList<int>>);
}

template <class L>
void slist_basic_impl() {
    if constexpr (kListReady<L>) {
        L l;
        for (int i = 5; i >= 1; --i) l.push_front(i);
        CHECK_EQ(l.size(), std::size_t{5});
        std::vector<int> seen;
        for (int x : l) seen.push_back(x);
        CHECK(seen == (std::vector<int>{1, 2, 3, 4, 5}));
        for (int& x : l) x *= 10;
        const L& cl = l;
        CHECK_EQ(*cl.begin(), 10);
        CHECK_EQ(std::count_if(cl.begin(), cl.end(), [](int x) { return x > 25; }), 3);
        auto it = std::find(l.begin(), l.end(), 30);
        CHECK(it != l.end());
        typename L::const_iterator cit = it;  // conversion
        CHECK(cit == it);                     // mixed comparison through conversion
        auto r = std::ranges::find(l, 40);
        CHECK(r != l.end());
    } else {
        require_ready<L>();
    }
}
TEST(slist_basic) { slist_basic_impl<SList<int>>(); }

template <class L>
void slist_insert_after_impl() {
    if constexpr (kListReady<L>) {
        L l;
        l.push_front("c");
        l.push_front("a");
        auto it = l.insert_after(l.cbegin(), "b");
        CHECK_EQ(*it, std::string("b"));
        it->append("!");
        l.insert_after(std::next(l.cbegin(), 2), "d");
        std::vector<std::string> seen(l.begin(), l.end());
        CHECK(seen == (std::vector<std::string>{"a", "b!", "c", "d"}));
        auto post = l.begin();
        auto old = post++;
        CHECK_EQ(*old, std::string("a"));
        CHECK_EQ(*post, std::string("b!"));
    } else {
        require_ready<L>();
    }
}
TEST(slist_insert_after) { slist_insert_after_impl<SList<std::string>>(); }

TEST(slist_empty) {
    SList<int> l;
    CHECK_EQ(l.size(), std::size_t{0});
    CHECK(l.begin() == l.end());
    CHECK(l.cbegin() == l.cend());
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
