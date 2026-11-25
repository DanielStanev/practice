#include "concept_overloading.hpp"

#include <forward_list>
#include <list>
#include <string>
#include <vector>

#include "testing.hpp"

using idioms::is_palindrome;
using idioms::last_n;
using idioms::middle;

namespace {

// Counting iterator over int[] with a configurable category (C++20 style).
template <class Tag>
struct Probe {
    using iterator_concept = Tag;
    using iterator_category = Tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;

    static constexpr bool kBidi = std::is_base_of_v<std::bidirectional_iterator_tag, Tag>;
    static constexpr bool kRandom = std::is_base_of_v<std::random_access_iterator_tag, Tag>;
    static inline long inc = 0, dec = 0;
    static void reset() { inc = dec = 0; }

    int* p = nullptr;
    int& operator*() const { return *p; }
    int& operator[](difference_type n) const requires kRandom { return p[n]; }
    Probe& operator++() { ++p; ++inc; return *this; }
    Probe operator++(int) { Probe t = *this; ++*this; return t; }
    Probe& operator--() requires kBidi { --p; ++dec; return *this; }
    Probe operator--(int) requires kBidi { Probe t = *this; --*this; return t; }
    Probe& operator+=(difference_type n) requires kRandom { p += n; return *this; }
    Probe& operator-=(difference_type n) requires kRandom { p -= n; return *this; }
    friend Probe operator+(Probe a, difference_type n) requires kRandom { return a += n; }
    friend Probe operator+(difference_type n, Probe a) requires kRandom { return a += n; }
    friend Probe operator-(Probe a, difference_type n) requires kRandom { return a -= n; }
    friend difference_type operator-(Probe a, Probe b) requires kRandom { return a.p - b.p; }
    friend bool operator==(Probe a, Probe b) { return a.p == b.p; }
    friend auto operator<=>(Probe a, Probe b) requires kRandom { return a.p <=> b.p; }
};

using Fwd = Probe<std::forward_iterator_tag>;
using Bidi = Probe<std::bidirectional_iterator_tag>;
using Rand = Probe<std::random_access_iterator_tag>;
static_assert(std::forward_iterator<Fwd> && !std::bidirectional_iterator<Fwd>);
static_assert(std::bidirectional_iterator<Bidi> && !std::random_access_iterator<Bidi>);
static_assert(std::random_access_iterator<Rand>);

int data[64];

template <class It>
concept PalindromeCallable = requires(It i) { is_palindrome(i, i); };

}  // namespace

TEST(middle_values) {
    for (int n = 1; n <= 9; ++n) {
        std::forward_list<int> fl;
        std::vector<int> v;
        for (int i = n - 1; i >= 0; --i) fl.push_front(i);
        for (int i = 0; i < n; ++i) v.push_back(i);
        CHECK_EQ(*middle(fl.begin(), fl.end()), n / 2);
        CHECK_EQ(*middle(v.begin(), v.end()), n / 2);
    }
    std::vector<int> empty;
    CHECK(middle(empty.begin(), empty.end()) == empty.end());
}

TEST(middle_forward_is_single_pass) {
    Fwd::reset();
    Fwd m = middle(Fwd{data}, Fwd{data + 40});
    CHECK(m.p == data + 20);
    CHECK(Fwd::inc <= 61);  // n + n/2 + 1
}

TEST(middle_random_access_no_steps) {
    Rand::reset();
    Rand m = middle(Rand{data}, Rand{data + 41});
    CHECK(m.p == data + 20);
    CHECK_EQ(Rand::inc, 0L);
}

TEST(last_n_values) {
    std::forward_list<int> fl{0, 1, 2, 3, 4, 5};
    std::list<int> l{0, 1, 2, 3, 4, 5};
    std::vector<int> v{0, 1, 2, 3, 4, 5};
    CHECK_EQ(*last_n(fl.begin(), fl.end(), 2), 4);
    CHECK_EQ(*last_n(l.begin(), l.end(), 2), 4);
    CHECK_EQ(*last_n(v.begin(), v.end(), 2), 4);
    CHECK(last_n(fl.begin(), fl.end(), 0) == fl.end());
    CHECK(last_n(l.begin(), l.end(), 0) == l.end());
    CHECK(last_n(v.begin(), v.end(), 0) == v.end());
    CHECK(last_n(fl.begin(), fl.end(), 100) == fl.begin());
    CHECK(last_n(l.begin(), l.end(), 100) == l.begin());
    CHECK(last_n(v.begin(), v.end(), 100) == v.begin());
}

TEST(last_n_forward_two_pointers) {
    Fwd::reset();
    Fwd r = last_n(Fwd{data}, Fwd{data + 50}, 5);
    CHECK(r.p == data + 45);
    CHECK(Fwd::inc <= 95);  // lead walks 50, lag walks 45
}

TEST(last_n_bidirectional_walks_back) {
    Bidi::reset();
    Bidi r = last_n(Bidi{data}, Bidi{data + 50}, 5);
    CHECK(r.p == data + 45);
    CHECK_EQ(Bidi::inc, 0L);
    CHECK_EQ(Bidi::dec, 5L);
}

TEST(last_n_random_access_constant) {
    Rand::reset();
    Rand r = last_n(Rand{data}, Rand{data + 50}, 5);
    CHECK(r.p == data + 45);
    CHECK_EQ(Rand::inc + Rand::dec, 0L);
}

TEST(palindromes) {
    std::string a = "racecar", b = "abba", c = "abca", d = "", e = "x";
    CHECK(is_palindrome(a.begin(), a.end()));
    CHECK(is_palindrome(b.begin(), b.end()));
    CHECK_FALSE(is_palindrome(c.begin(), c.end()));
    CHECK(is_palindrome(d.begin(), d.end()));
    CHECK(is_palindrome(e.begin(), e.end()));
    std::list<int> l{1, 2, 3, 2, 1};
    CHECK(is_palindrome(l.begin(), l.end()));
}

TEST(palindrome_rejects_forward_iterators) {
    CHECK_TRAIT(PalindromeCallable<std::list<int>::iterator>);
    CHECK_TRAIT(!PalindromeCallable<std::forward_list<int>::iterator>);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
