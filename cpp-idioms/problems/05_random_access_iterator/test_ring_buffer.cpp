#include "ring_buffer.hpp"

#include <algorithm>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

#include "testing.hpp"

using idioms::RingBuffer;

namespace {

template <class B>
constexpr bool kReady = std::random_access_iterator<typename B::iterator> &&
                        std::random_access_iterator<typename B::const_iterator> &&
                        requires(B& b, const B& cb) {
                            b.rbegin();
                            b.rend();
                            cb.rbegin();
                        };

// Fills an 8-slot buffer so that the logical sequence wraps physically:
// push 6, pop 6, push values (head is now at physical slot 6). Result: logical contents == values.
template <class B, class V>
void fill_wrapped(B& b, const V& values) {
    for (int i = 0; i < 6; ++i) b.push_back(typename B::value_type{});
    for (int i = 0; i < 6; ++i) b.pop_front();
    for (const auto& v : values) b.push_back(v);
}

}  // namespace

TEST(iterators_are_random_access) {
    using B = RingBuffer<int, 8>;
    CHECK_TRAIT(std::random_access_iterator<B::iterator>);
    CHECK_TRAIT(std::random_access_iterator<B::const_iterator>);
    CHECK_TRAIT(std::is_convertible_v<B::iterator, B::const_iterator>);
    CHECK_TRAIT(!std::is_convertible_v<B::const_iterator, B::iterator>);
    CHECK_TRAIT(std::ranges::random_access_range<B>);
    CHECK_TRAIT(std::ranges::random_access_range<const B>);
}

TEST(fifo_basics) {
    RingBuffer<int, 3> b;
    CHECK(b.empty());
    CHECK(b.push_back(1));
    CHECK(b.push_back(2));
    CHECK(b.push_back(3));
    CHECK(b.full());
    CHECK_FALSE(b.push_back(4));
    CHECK_EQ(b.front(), 1);
    b.pop_front();
    CHECK(b.push_back(4));
    CHECK_EQ(b.size(), std::size_t{3});
    CHECK_EQ(b[0], 2);
    CHECK_EQ(b[1], 3);
    CHECK_EQ(b[2], 4);
    CHECK_EQ(b.back(), 4);
}

template <class B>
void sort_wrapped_impl() {
    if constexpr (kReady<B>) {
        B b;
        fill_wrapped(b, std::vector<int>{5, 1, 4, 2, 3, 0});
        std::sort(b.begin(), b.end());
        for (int i = 0; i < 6; ++i) CHECK_EQ(b[i], i);
        std::ranges::sort(b, std::greater<>{});
        CHECK_EQ(b.front(), 5);
        CHECK_EQ(b.back(), 0);
    } else {
        ::testing::fail(__FILE__, __LINE__, "iterators are not random access yet");
    }
}
TEST(sort_wrapped) { sort_wrapped_impl<RingBuffer<int, 8>>(); }

template <class B>
void arithmetic_impl() {
    if constexpr (kReady<B>) {
        B b;
        fill_wrapped(b, std::vector<int>{10, 20, 30, 40, 50});
        auto it = b.begin();
        CHECK_EQ(it[3], 40);
        CHECK_EQ(*(it + 2), 30);
        CHECK_EQ(*(2 + it), 30);
        auto e = b.end();
        CHECK_EQ(e - it, 5);
        CHECK_EQ(*(e - 1), 50);
        CHECK(it < e);
        CHECK(e > it);
        CHECK(it <= it);
        it += 4;
        CHECK_EQ(*it, 50);
        it -= 3;
        CHECK_EQ(*it, 20);
        CHECK_EQ(*it--, 20);
        CHECK_EQ(*it, 10);
        CHECK_EQ(*++it, 20);
        typename B::const_iterator c = it;
        CHECK_EQ(*c, 20);
        *it = 21;
        CHECK_EQ(b[1], 21);
    } else {
        ::testing::fail(__FILE__, __LINE__, "iterators are not random access yet");
    }
}
TEST(iterator_arithmetic) { arithmetic_impl<RingBuffer<int, 8>>(); }

template <class B>
void algorithms_impl() {
    if constexpr (kReady<B>) {
        B b;
        fill_wrapped(b, std::vector<std::string>{"ask", "bid", "fill", "quote", "trade"});
        const B& cb = b;
        auto lb = std::lower_bound(cb.begin(), cb.end(), std::string("cancel"));
        CHECK_EQ(lb - cb.begin(), 2);
        CHECK(std::binary_search(cb.begin(), cb.end(), std::string("quote")));
        std::vector<std::string> rev(cb.rbegin(), cb.rend());
        CHECK(rev == (std::vector<std::string>{"trade", "quote", "fill", "bid", "ask"}));
        std::reverse(b.begin(), b.end());
        CHECK_EQ(b.front(), std::string("trade"));
        auto mid = b.begin() + 2;
        std::nth_element(b.begin(), mid, b.end());
        CHECK_EQ(*mid, std::string("fill"));
        CHECK_EQ(std::ranges::distance(b), 5);
    } else {
        ::testing::fail(__FILE__, __LINE__, "iterators are not random access yet");
    }
}
TEST(standard_algorithms) { algorithms_impl<RingBuffer<std::string, 8>>(); }

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
