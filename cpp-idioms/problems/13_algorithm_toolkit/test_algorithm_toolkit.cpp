#include "algorithm_toolkit.hpp"

#include <random>
#include <vector>

#include "testing.hpp"

using namespace idioms;

TEST(top_k_values) {
    CHECK(top_k({3, 9, 1, 7, 5}, 3) == (std::vector<double>{9, 7, 5}));
    CHECK(top_k({3, 9}, 5) == (std::vector<double>{9, 3}));
    CHECK(top_k({}, 2).empty());
    CHECK(top_k({1, 2}, 0).empty());
}

TEST(percentile_values) {
    std::vector<long> v;
    for (long i = 1; i <= 101; ++i) v.push_back(102 - i);  // 101..1 reversed
    CHECK_EQ(percentile(v, 0.5), 51L);
    CHECK_EQ(percentile(v, 0.0), 1L);
    CHECK_EQ(percentile(v, 1.0), 101L);
    CHECK_EQ(percentile(v, 0.99), 100L);
    CHECK_EQ(percentile({42}, 0.9), 42L);
    std::mt19937 rng(1);
    std::vector<long> big(100001);
    std::iota(big.begin(), big.end(), 0L);
    std::shuffle(big.begin(), big.end(), rng);
    CHECK_EQ(percentile(big, 0.999), 99900L);
}

TEST(vwap_values) {
    CHECK_NEAR(vwap({{100.0, 10}, {101.0, 30}}), 100.75, 1e-12);
    CHECK_NEAR(vwap({{50.0, 1}}), 50.0, 1e-12);
    CHECK_EQ(vwap({}), 0.0);
    CHECK_EQ(vwap({{10.0, 0}}), 0.0);
}

TEST(urgent_first_stable) {
    std::vector<Order> v{{1, false}, {2, true}, {3, false}, {4, true}, {5, true}, {6, false}};
    urgent_first(v);
    std::vector<int> ids;
    for (auto& o : v) ids.push_back(o.id);
    CHECK(ids == (std::vector<int>{2, 4, 5, 1, 3, 6}));
}

TEST(first_gap_values) {
    CHECK_EQ(first_gap({100.0, 100.5, 101.0, 105.0, 105.5}, 1.0), 2L);
    CHECK_EQ(first_gap({100.0, 99.0, 97.0}, 1.5), 1L);  // drops count too
    CHECK_EQ(first_gap({1.0, 1.1, 1.2}, 1.0), -1L);
    CHECK_EQ(first_gap({}, 1.0), -1L);
    CHECK_EQ(first_gap({5.0}, 1.0), -1L);
}

TEST(move_to_front_values) {
    std::vector<int> v{1, 2, 3, 4, 5};
    move_to_front(v, 4);
    CHECK(v == (std::vector<int>{4, 1, 2, 3, 5}));
    move_to_front(v, 4);
    CHECK(v == (std::vector<int>{4, 1, 2, 3, 5}));
    move_to_front(v, 5);
    CHECK(v == (std::vector<int>{5, 4, 1, 2, 3}));
    move_to_front(v, 99);
    CHECK(v == (std::vector<int>{5, 4, 1, 2, 3}));
}

TEST(running_position_values) {
    CHECK(running_position({100, -30, -80, 10}) == (std::vector<long>{100, 70, -10, 0}));
    CHECK(running_position({}).empty());
}

TEST(dedupe_sorted_values) {
    std::vector<int> v{1, 1, 2, 3, 3, 3, 7, 7};
    dedupe_sorted(v);
    CHECK(v == (std::vector<int>{1, 2, 3, 7}));
    std::vector<int> e;
    dedupe_sorted(e);
    CHECK(e.empty());
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
