#include "fibonacci.hpp"

#include <cstdint>

#include "testing.hpp"

namespace {
// Independent reference values, not computed by any function under test.
constexpr std::uint64_t kFib[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610};
}  // namespace

TEST(memo_small_values) {
    for (int i = 0; i < 16; ++i) CHECK_EQ(dp::fib_memo(i), kFib[i]);
}

TEST(table_small_values) {
    for (int i = 0; i < 16; ++i) CHECK_EQ(dp::fib_table(i), kFib[i]);
}

TEST(two_vars_small_values) {
    for (int i = 0; i < 16; ++i) CHECK_EQ(dp::fib_two_vars(i), kFib[i]);
}

TEST(large_values) {
    CHECK_EQ(dp::fib_memo(50), 12586269025ULL);
    CHECK_EQ(dp::fib_table(90), 2880067194370816120ULL);
    CHECK_EQ(dp::fib_two_vars(93), 12200160415121876738ULL);
    CHECK_EQ(dp::fib_memo(93), 12200160415121876738ULL);
    CHECK_EQ(dp::fib_table(93), 12200160415121876738ULL);
}

TEST(all_methods_agree) {
    for (int n = 0; n <= 93; ++n) {
        auto a = dp::fib_memo(n);
        CHECK_EQ(dp::fib_table(n), a);
        CHECK_EQ(dp::fib_two_vars(n), a);
    }
}

TEST(memo_is_fast) {
    // A naive recursion would need ~10^19 calls for n = 93. Memoized, this is instant.
    std::uint64_t sink = 0;
    for (int rep = 0; rep < 2000; ++rep) sink += dp::fib_memo(93 - rep % 3);
    CHECK(sink != 0);
}

TEST(naive_call_count_small) {
    CHECK_EQ(dp::naive_call_count(0), 1ULL);
    CHECK_EQ(dp::naive_call_count(1), 1ULL);
    CHECK_EQ(dp::naive_call_count(2), 3ULL);
    CHECK_EQ(dp::naive_call_count(3), 5ULL);
    CHECK_EQ(dp::naive_call_count(5), 15ULL);
}

TEST(naive_call_count_matches_real_recursion) {
    // Count the calls of the real recursion for small n and compare.
    for (int n = 0; n <= 20; ++n) {
        std::uint64_t calls = 0;
        auto go = [&](auto& self, int k) -> std::uint64_t {
            ++calls;
            return k < 2 ? static_cast<std::uint64_t>(k) : self(self, k - 1) + self(self, k - 2);
        };
        go(go, n);
        CHECK_EQ(dp::naive_call_count(n), calls);
    }
}

TEST(naive_call_count_large) {
    CHECK_EQ(dp::naive_call_count(30), 2692537ULL);
    CHECK_EQ(dp::naive_call_count(70), 616123042340257ULL);
}

TEST(tribonacci_values) {
    constexpr std::uint64_t kTrib[] = {0, 1, 1, 2, 4, 7, 13, 24, 44, 81, 149};
    for (int i = 0; i <= 10; ++i) CHECK_EQ(dp::tribonacci(i), kTrib[i]);
    CHECK_EQ(dp::tribonacci(25), 1389537ULL);
    CHECK_EQ(dp::tribonacci(37), 2082876103ULL);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
