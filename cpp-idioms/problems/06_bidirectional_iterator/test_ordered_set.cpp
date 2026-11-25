#include "ordered_set.hpp"

#include <pthread.h>

#include <algorithm>
#include <random>
#include <ranges>
#include <set>
#include <string>
#include <vector>

#include "testing.hpp"

using idioms::OrderedSet;

namespace {
template <class S>
constexpr bool kReady = std::bidirectional_iterator<typename S::iterator> &&
                        requires(const S& s) { s.rbegin(); s.rend(); };
#define REQUIRE_READY(S)                                                                   \
    if constexpr (!kReady<S>) {                                                            \
        ::testing::fail(__FILE__, __LINE__, "iterator is not bidirectional yet");          \
    } else
}  // namespace

TEST(iterator_is_bidirectional) {
    using S = OrderedSet<int>;
    CHECK_TRAIT(std::bidirectional_iterator<S::iterator>);
    CHECK_TRAIT(std::ranges::bidirectional_range<S>);
    CHECK_TRAIT(std::is_same_v<std::iter_reference_t<S::iterator>, const int&>);
}

TEST(insert_and_find) {
    OrderedSet<int> s;
    CHECK(s.insert(50).second);
    CHECK(s.insert(30).second);
    CHECK(s.insert(70).second);
    CHECK_FALSE(s.insert(30).second);
    CHECK_EQ(*s.insert(30).first, 30);
    CHECK_EQ(s.size(), std::size_t{3});
    CHECK(s.find(70) != s.end());
    CHECK_EQ(*s.find(70), 70);
    CHECK(s.find(60) == s.end());
    CHECK_EQ(*s.lower_bound(60), 70);
    CHECK_EQ(*s.lower_bound(30), 30);
    CHECK(s.lower_bound(71) == s.end());
}

template <class S>
void in_order_impl() {
    REQUIRE_READY(S) {
        S s;
        for (int k : {50, 30, 70, 20, 40, 80, 60, 10, 45}) s.insert(k);
        std::vector<int> fwd(s.begin(), s.end());
        CHECK(fwd == (std::vector<int>{10, 20, 30, 40, 45, 50, 60, 70, 80}));
        std::vector<int> back(s.rbegin(), s.rend());
        CHECK(back == (std::vector<int>{80, 70, 60, 50, 45, 40, 30, 20, 10}));
        CHECK_EQ(*std::prev(s.end()), 80);
        auto it = s.find(45);
        CHECK_EQ(*std::prev(it), 40);
        CHECK_EQ(*std::next(it), 50);
        CHECK_EQ(*it++, 45);
        CHECK_EQ(*it--, 50);
        CHECK_EQ(*it, 45);
        CHECK_EQ(std::ranges::distance(s), 9);
        std::vector<int> rv;
        for (int x : s | std::views::reverse | std::views::take(3)) rv.push_back(x);
        CHECK(rv == (std::vector<int>{80, 70, 60}));
    }
}
TEST(in_order_both_directions) { in_order_impl<OrderedSet<int>>(); }

template <class S>
void stability_impl() {
    REQUIRE_READY(S) {
        S s;
        auto keep = s.insert("m").first;
        for (char c = 'a'; c <= 'z'; ++c) s.insert(std::string(1, c));
        CHECK_EQ(*keep, std::string("m"));  // still valid after many inserts
        CHECK_EQ(*std::next(keep), std::string("n"));
        CHECK_EQ(keep->size(), std::size_t{1});
    }
}
TEST(iterators_survive_inserts) { stability_impl<OrderedSet<std::string>>(); }

template <class S>
void random_vs_std_impl() {
    REQUIRE_READY(S) {
        std::mt19937 rng(7);
        S s;
        std::set<int> ref;
        for (int i = 0; i < 3000; ++i) {
            int k = static_cast<int>(rng() % 1000);
            CHECK_EQ(s.insert(k).second, ref.insert(k).second);
        }
        CHECK(std::equal(s.begin(), s.end(), ref.begin(), ref.end()));
        CHECK(std::equal(s.rbegin(), s.rend(), ref.rbegin(), ref.rend()));
    }
}
TEST(matches_std_set) { random_vs_std_impl<OrderedSet<int>>(); }

// Builds and destroys a 20000-deep tree on a thread with a 256 KiB stack, so
// a recursive destructor overflows the stack and crashes the test binary.
TEST(deep_tree_destructor) {
    struct Job {
        static void* run(void* out) {
            try {
                OrderedSet<int> s;
                for (int i = 0; i < 20000; ++i) s.insert(i);
                *static_cast<std::size_t*>(out) = s.size();
            } catch (...) {  // exceptions must not escape a thread
            }
            return nullptr;
        }
    };
    std::size_t size = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 256 * 1024);
    pthread_t t;
    CHECK_EQ(pthread_create(&t, &attr, &Job::run, &size), 0);
    pthread_join(t, nullptr);
    pthread_attr_destroy(&attr);
    CHECK_EQ(size, std::size_t{20000});
}

TEST(empty_set) {
    OrderedSet<int> s;
    CHECK(s.begin() == s.end());
    CHECK(s.find(1) == s.end());
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
