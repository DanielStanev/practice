#include "climbing_stairs.hpp"

#include <cstdint>
#include <vector>

#include "testing.hpp"

namespace {

// Brute force: try every sequence of moves recursively. Only for small n.
std::uint64_t brute(int remaining, const std::vector<int>& steps) {
    if (remaining == 0) return 1;
    std::uint64_t total = 0;
    for (int s : steps)
        if (s <= remaining) total += brute(remaining - s, steps);
    return total;
}

std::uint64_t brute_no_double_two(int remaining, bool last_was_two) {
    if (remaining == 0) return 1;
    std::uint64_t total = brute_no_double_two(remaining - 1, false);
    if (remaining >= 2 && !last_was_two) total += brute_no_double_two(remaining - 2, true);
    return total;
}

}  // namespace

TEST(count_ways_base_cases) {
    CHECK_EQ(dp::count_ways(0), 1ULL);
    CHECK_EQ(dp::count_ways(1), 1ULL);
    CHECK_EQ(dp::count_ways(2), 2ULL);
    CHECK_EQ(dp::count_ways(3), 3ULL);
}

TEST(count_ways_matches_brute_force) {
    for (int n = 0; n <= 25; ++n) CHECK_EQ(dp::count_ways(n), brute(n, {1, 2}));
}

TEST(count_ways_large) {
    CHECK_EQ(dp::count_ways(45), 1836311903ULL);
    CHECK_EQ(dp::count_ways(90), 4660046610375530309ULL);
}

TEST(with_steps_base_cases) {
    CHECK_EQ(dp::count_ways_with_steps(0, {1, 2}), 1);
    CHECK_EQ(dp::count_ways_with_steps(1, {2}), 0);
    CHECK_EQ(dp::count_ways_with_steps(2, {2}), 1);
    CHECK_EQ(dp::count_ways_with_steps(4, {1, 3, 5}), 3);  // 1111, 13, 31
}

TEST(with_steps_matches_brute_force) {
    std::vector<std::vector<int>> sets = {{1}, {1, 2}, {1, 2, 3}, {2, 3}, {1, 3, 5}, {4, 7}, {3, 1, 2, 6}};
    for (const auto& steps : sets)
        for (int n = 0; n <= 22; ++n)
            CHECK_EQ(dp::count_ways_with_steps(n, steps), static_cast<std::int64_t>(brute(n, steps)));
}

TEST(with_steps_matches_count_ways_mod) {
    for (int n = 0; n <= 90; ++n)
        CHECK_EQ(dp::count_ways_with_steps(n, {1, 2}),
                 static_cast<std::int64_t>(dp::count_ways(n) % dp::kMod));
}

TEST(with_steps_large_uses_modulo) {
    CHECK_EQ(dp::count_ways_with_steps(1000, {1, 2}), 107579939);
    CHECK_EQ(dp::count_ways_with_steps(200000, {1, 2, 3, 5, 8, 13, 21, 34, 55, 89}), 106499004);
}

TEST(no_double_two_small) {
    CHECK_EQ(dp::count_ways_no_double_two(0), 1ULL);
    CHECK_EQ(dp::count_ways_no_double_two(1), 1ULL);
    CHECK_EQ(dp::count_ways_no_double_two(2), 2ULL);
    CHECK_EQ(dp::count_ways_no_double_two(4), 4ULL);  // 1111, 112, 121, 211 (22 is banned)
}

TEST(no_double_two_matches_brute_force) {
    for (int n = 0; n <= 30; ++n) CHECK_EQ(dp::count_ways_no_double_two(n), brute_no_double_two(n, false));
}

TEST(no_double_two_large) {
    CHECK_EQ(dp::count_ways_no_double_two(80), 17098272199297ULL);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
