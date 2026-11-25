#include "output_iterators.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "alloc_counter.hpp"
#include "testing.hpp"

using namespace idioms;

namespace {
struct Collect {
    std::vector<std::string>* out;
    void operator()(const std::string& s) const { out->push_back(s); }
};
template <class It, class T>
constexpr bool kOut = std::output_iterator<It, T>;
}  // namespace

TEST(concepts_hold) {
    CHECK_TRAIT(std::output_iterator<FunctionOutput<Collect>, std::string>);
    CHECK_TRAIT(std::output_iterator<FunctionOutput<Collect>, const char*>);
    CHECK_TRAIT(std::output_iterator<CountingOutput, int>);
    CHECK_TRAIT(std::output_iterator<CountingOutput, std::string>);
}

template <class It>
void function_output_impl() {
    if constexpr (kOut<It, std::string>) {
        std::vector<std::string> got;
        std::vector<std::string> src{"a", "bb", "ccc"};
        std::copy(src.begin(), src.end(), It(Collect{&got}));
        CHECK(got == src);
        int calls = 0;
        auto counter = function_output([&calls, n = 0](int) mutable { ++n; calls = n; });
        std::fill_n(counter, 5, 0);
        CHECK_EQ(calls, 5);
    } else {
        ::testing::fail(__FILE__, __LINE__, "FunctionOutput is not an output iterator yet");
    }
}
TEST(function_output_forwards) { function_output_impl<FunctionOutput<Collect>>(); }

template <class It>
void counting_impl() {
    if constexpr (kOut<It, int>) {
        std::vector<int> v{1, 2, 2, 3, 3, 3};
        auto out = std::unique_copy(v.begin(), v.end(), It{});
        CHECK_EQ(out.count(), std::size_t{3});
        CHECK_EQ(It{}.count(), std::size_t{0});
    } else {
        ::testing::fail(__FILE__, __LINE__, "CountingOutput is not an output iterator yet");
    }
}
TEST(counting_output_counts) { counting_impl<CountingOutput>(); }

TEST(intersection_size_no_alloc) {
    std::vector<int> a, b;
    for (int i = 0; i < 1000; ++i) a.push_back(i * 2);
    for (int i = 0; i < 1000; ++i) b.push_back(i * 3);
    long before = alloc_counter::count();
    std::size_t n = intersection_size(a, b);
    CHECK_EQ(alloc_counter::count(), before);
    CHECK_EQ(n, std::size_t{334});  // multiples of 6 below 1998
    CHECK_EQ(intersection_size({}, {1}), std::size_t{0});
}

TEST(split_fills_partition_copy) {
    std::vector<Fill> fills{{1, 100}, {2, 5}, {3, 50}, {4, 49}, {5, 500}};
    std::vector<Fill> big{{0, 999}}, small;
    split_fills(fills, big, small, 50);
    CHECK_EQ(big.size(), std::size_t{4});
    CHECK_EQ(big[0].id, 0);  // appended, existing contents kept
    CHECK_EQ(big[1].id, 1);
    CHECK_EQ(big[2].id, 3);
    CHECK_EQ(big[3].id, 5);
    CHECK_EQ(small.size(), std::size_t{2});
    CHECK_EQ(small[0].id, 2);
    CHECK_EQ(small[1].id, 4);
}

TEST(sum_of_squares_values) {
    CHECK_EQ(sum_of_squares({1, 2, 3}), 14L);
    CHECK_EQ(sum_of_squares({}), 0L);
    CHECK_EQ(sum_of_squares({100000, 100000}), 20000000000L);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
