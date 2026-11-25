#include "iterator_categories.hpp"

#include <deque>
#include <forward_list>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "testing.hpp"

using idioms::category_name;
using idioms::my_advance;
using idioms::my_distance;

namespace {

// An iterator over int[] that reports the category `Tag` and counts every step.
template <class Tag>
struct Probe {
    using iterator_category = Tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = int*;
    using reference = int&;

    static constexpr bool kBidi = std::is_base_of_v<std::bidirectional_iterator_tag, Tag>;
    static constexpr bool kRandom = std::is_base_of_v<std::random_access_iterator_tag, Tag>;

    static inline long increments = 0;
    static inline long decrements = 0;
    static inline long jumps = 0;
    static void reset() { increments = decrements = jumps = 0; }

    int* p = nullptr;

    int& operator*() const { return *p; }
    Probe& operator++() { ++p; ++increments; return *this; }
    Probe operator++(int) { Probe t = *this; ++*this; return t; }
    Probe& operator--() requires kBidi { --p; ++decrements; return *this; }
    Probe operator--(int) requires kBidi { Probe t = *this; --*this; return t; }
    Probe& operator+=(difference_type n) requires kRandom { p += n; ++jumps; return *this; }
    Probe& operator-=(difference_type n) requires kRandom { p -= n; ++jumps; return *this; }
    friend Probe operator+(Probe a, difference_type n) requires kRandom { return a += n; }
    friend Probe operator-(Probe a, difference_type n) requires kRandom { return a -= n; }
    friend difference_type operator-(Probe a, Probe b) requires kRandom { ++jumps; return a.p - b.p; }
    friend bool operator==(Probe a, Probe b) { return a.p == b.p; }
};

using FwdProbe = Probe<std::forward_iterator_tag>;
using BidiProbe = Probe<std::bidirectional_iterator_tag>;
using RandProbe = Probe<std::random_access_iterator_tag>;

int data[100];

}  // namespace

TEST(distance_random_access_is_constant_time) {
    RandProbe::reset();
    long d = my_distance(RandProbe{data}, RandProbe{data + 100});
    CHECK_EQ(d, 100L);
    CHECK_EQ(RandProbe::increments, 0L);
}

TEST(distance_forward_walks) {
    FwdProbe::reset();
    CHECK_EQ(my_distance(FwdProbe{data + 10}, FwdProbe{data + 40}), 30L);
    CHECK_EQ(FwdProbe::increments, 30L);
}

TEST(distance_bidirectional_walks) {
    BidiProbe::reset();
    CHECK_EQ(my_distance(BidiProbe{data}, BidiProbe{data + 7}), 7L);
    CHECK_EQ(BidiProbe::increments, 7L);
}

TEST(distance_on_real_containers) {
    std::vector<int> v(17);
    std::list<int> l(9);
    std::forward_list<int> fl(4);
    std::set<int> s{1, 2, 3};
    CHECK_EQ(my_distance(v.begin(), v.end()), 17L);
    CHECK_EQ(my_distance(l.begin(), l.end()), 9L);
    CHECK_EQ(my_distance(fl.begin(), fl.end()), 4L);
    CHECK_EQ(my_distance(s.begin(), s.end()), 3L);
    CHECK_EQ(my_distance(v.data(), v.data() + 5), 5L);  // raw pointers too
}

TEST(distance_input_iterator) {
    std::istringstream in("1 2 3 4 5");
    CHECK_EQ(my_distance(std::istream_iterator<int>(in), std::istream_iterator<int>()), 5L);
}

TEST(advance_random_access_jumps_once) {
    RandProbe::reset();
    RandProbe it{data};
    my_advance(it, 60);
    CHECK(it.p == data + 60);
    my_advance(it, -25);
    CHECK(it.p == data + 35);
    CHECK_EQ(RandProbe::jumps, 2L);
    CHECK_EQ(RandProbe::increments + RandProbe::decrements, 0L);
}

TEST(advance_bidirectional_both_ways) {
    BidiProbe::reset();
    BidiProbe it{data + 50};
    my_advance(it, 5);
    CHECK(it.p == data + 55);
    my_advance(it, -20);
    CHECK(it.p == data + 35);
    CHECK_EQ(BidiProbe::increments, 5L);
    CHECK_EQ(BidiProbe::decrements, 20L);
}

TEST(advance_forward_negative_throws) {
    FwdProbe it{data + 10};
    my_advance(it, 3);
    CHECK(it.p == data + 13);
    CHECK_THROWS_AS(my_advance(it, -1), std::invalid_argument);
}

TEST(advance_real_containers) {
    std::map<int, char> m{{1, 'a'}, {2, 'b'}, {3, 'c'}};
    auto it = m.begin();
    my_advance(it, 2);
    CHECK_EQ(it->second, 'c');
    my_advance(it, -1);
    CHECK_EQ(it->second, 'b');
    std::deque<int> d{0, 1, 2, 3, 4, 5};
    auto di = d.begin();
    my_advance(di, 4);
    CHECK_EQ(*di, 4);
}

TEST(category_names) {
    CHECK_EQ(category_name<std::vector<int>::iterator>(), "contiguous");
    CHECK_EQ(category_name<const char*>(), "contiguous");
    CHECK_EQ(category_name<std::deque<int>::iterator>(), "random_access");
    CHECK_EQ(category_name<std::list<int>::iterator>(), "bidirectional");
    CHECK_EQ((category_name<std::map<int, int>::const_iterator>()), "bidirectional");
    CHECK_EQ(category_name<std::forward_list<int>::iterator>(), "forward");
    CHECK_EQ(category_name<std::istream_iterator<int>>(), "input");
    CHECK_EQ(category_name<std::back_insert_iterator<std::vector<int>>>(), "output");
    CHECK_EQ(category_name<std::ostream_iterator<int>>(), "output");
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
