#include "custom_views.hpp"

#include <forward_list>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
using VecView = std::ranges::ref_view<std::vector<int>>;

template <class R>
auto to_vector(R&& r) {
    std::vector<std::ranges::range_value_t<R>> out;
    for (auto&& x : r) out.push_back(x);
    return out;
}

// Makes T a dependent type, so code in a discarded `if constexpr` branch is
// not checked until the view is actually ready.
template <class D, class T>
using dep_t = std::conditional_t<std::is_void_v<D>, T, T>;

template <class V>
constexpr bool kStrideReady = std::ranges::forward_range<StrideView<V>> && std::ranges::view<StrideView<V>>;
template <class V>
constexpr bool kPairReady = std::ranges::forward_range<PairwiseView<V>> && std::ranges::view<PairwiseView<V>>;
}  // namespace

TEST(concepts) {
    CHECK_TRAIT(std::ranges::view<StrideView<VecView>>);
    CHECK_TRAIT(std::ranges::forward_range<StrideView<VecView>>);
    CHECK_TRAIT(std::ranges::view<PairwiseView<VecView>>);
    CHECK_TRAIT(std::ranges::forward_range<PairwiseView<VecView>>);
}

template <class V>
void stride_impl() {
    if constexpr (kStrideReady<V>) {
        dep_t<V, std::vector<int>> v{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        CHECK(to_vector(v | stride(3)) == (std::vector<int>{0, 3, 6, 9}));
        CHECK(to_vector(v | stride(4)) == (std::vector<int>{0, 4, 8}));
        CHECK(to_vector(v | stride(1)) == v);
        CHECK(to_vector(v | stride(100)) == (std::vector<int>{0}));
        dep_t<V, std::vector<int>> empty;
        CHECK(to_vector(empty | stride(2)).empty());
        // composes after standard adaptors
        auto odd_then_stride = v | std::views::filter([](int x) { return x % 2 == 1; }) | stride(2);
        CHECK(to_vector(odd_then_stride) == (std::vector<int>{1, 5, 9}));
        // and before them
        auto squares = v | stride(5) | std::views::transform([](int x) { return x * x; });
        CHECK(to_vector(squares) == (std::vector<int>{0, 25}));
        // writes go through to the base
        for (int& x : v | stride(5)) x = -1;
        CHECK_EQ(v[5], -1);
        // forward_list (forward only) and a temporary (owning view)
        dep_t<V, std::forward_list<std::string>> fl{"a", "b", "c", "d", "e"};
        CHECK(to_vector(fl | stride(2)) == (std::vector<std::string>{"a", "c", "e"}));
        auto owned = dep_t<V, std::vector<int>>{10, 11, 12, 13} | stride(2);
        CHECK(to_vector(owned) == (std::vector<int>{10, 12}));
        CHECK_FALSE(owned.empty());  // from view_interface
        CHECK_EQ(owned.front(), 10);
    } else {
        ::testing::fail(__FILE__, __LINE__, "StrideView is not a forward view yet");
    }
}
TEST(stride_values) { stride_impl<VecView>(); }

template <class V>
void pairwise_impl() {
    if constexpr (kPairReady<V>) {
        dep_t<V, std::vector<int>> v{1, 2, 4, 7};
        auto pairs = to_vector(v | pairwise);
        CHECK_EQ(pairs.size(), std::size_t{3});
        CHECK(pairs[0] == std::make_pair(1, 2));
        CHECK(pairs[2] == std::make_pair(4, 7));
        dep_t<V, std::vector<int>> one{5}, none;
        CHECK(to_vector(one | pairwise).empty());
        CHECK(to_vector(none | pairwise).empty());
        auto diffs = v | pairwise | std::views::transform([](auto p) { return p.second - p.first; });
        CHECK(to_vector(diffs) == (std::vector<int>{1, 2, 3}));
        dep_t<V, std::forward_list<int>> fl{3, 1, 4};
        CHECK_EQ(std::ranges::distance(fl | pairwise), 2);
        CHECK_EQ(std::ranges::distance(v | stride(2) | pairwise), 1);  // {1,4} -> (1,4)
    } else {
        ::testing::fail(__FILE__, __LINE__, "PairwiseView is not a forward view yet");
    }
}
TEST(pairwise_values) { pairwise_impl<VecView>(); }

TEST(spreads_values) {
    auto s = spreads({100.0, 100.5, 100.25, 101.0});
    CHECK_EQ(s.size(), std::size_t{3});
    CHECK_NEAR(s[0], 0.5, 1e-12);
    CHECK_NEAR(s[1], 0.25, 1e-12);
    CHECK_NEAR(s[2], 0.75, 1e-12);
    CHECK(spreads({1.0}).empty());
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
