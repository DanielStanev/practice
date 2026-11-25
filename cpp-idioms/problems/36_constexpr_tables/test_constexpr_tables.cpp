#include "constexpr_tables.hpp"

#include <cstdint>
#include <string>
#include <string_view>

#include "testing.hpp"

using namespace idioms;

namespace {
// ConstexprOk<L> is true when the lambda L's body is a constant expression.
// An unfinished (throwing) implementation makes it false instead of breaking
// the build, so each test can fail on its own.
template <auto V>
struct Const {
    static constexpr auto value = V;
};
template <class L>
concept ConstexprOk = requires { Const<L{}()>::value; };

template <class L, class E>
void check_constexpr_eq(L, const E& expected, const char* expr, const char* file, int line) {
    if constexpr (ConstexprOk<L>) {
        ::testing::check_eq(Const<L{}()>::value, expected, expr, "expected", file, line);
    } else {
        ::testing::fail(file, line, std::string("not a constant expression: ") + expr);
    }
}
#define CHECK_CONSTEXPR_EQ(expr, expected) \
    check_constexpr_eq([] { return (expr); }, (expected), #expr, __FILE__, __LINE__)
}  // namespace

TEST(pow10) {
    CHECK_CONSTEXPR_EQ(kPow10[0], std::uint64_t{1});
    CHECK_CONSTEXPR_EQ(kPow10[6], std::uint64_t{1000000});
    CHECK_CONSTEXPR_EQ(kPow10[19], std::uint64_t{10000000000000000000ULL});
}

TEST(crc32_values) {
    CHECK_CONSTEXPR_EQ(crc32("123456789"), std::uint32_t{0xCBF43926u});
    CHECK_CONSTEXPR_EQ(crc32(""), std::uint32_t{0});
    CHECK_CONSTEXPR_EQ(crc32("The quick brown fox jumps over the lazy dog"), std::uint32_t{0x414FA339u});
    std::string_view runtime = "123456789";  // also works at run time
    CHECK_EQ(crc32(runtime), std::uint32_t{0xCBF43926u});
}

TEST(price_to_ticks_values) {
    CHECK_CONSTEXPR_EQ(price_to_ticks("123.45", 4), 1234500L);
    CHECK_CONSTEXPR_EQ(price_to_ticks("7", 2), 700L);
    CHECK_CONSTEXPR_EQ(price_to_ticks("0.0001", 4), 1L);
    CHECK_CONSTEXPR_EQ(price_to_ticks("99.", 1), 990L);
    // Invalid input cannot be tested here: with consteval, price_to_ticks("1.234", 2)
    // is a compile error at the call site. Try it yourself and read the message.
}

TEST(primes) {
    CHECK_CONSTEXPR_EQ(primes_below<30>().size(), std::size_t{10});
    CHECK_CONSTEXPR_EQ(primes_below<30>()[9], 29);
    CHECK_CONSTEXPR_EQ(primes_below<2>().size(), std::size_t{0});
    CHECK_CONSTEXPR_EQ(primes_below<1000>().size(), std::size_t{168});
}

TEST(char_class) {
    CHECK_CONSTEXPR_EQ(kCharClass['7'], kDigit);
    CHECK_CONSTEXPR_EQ(kCharClass['q'], kAlpha);
    CHECK_CONSTEXPR_EQ(kCharClass['Z'], kAlpha);
    CHECK_CONSTEXPR_EQ(kCharClass['\t'], kSpace);
    CHECK_CONSTEXPR_EQ(kCharClass['.'], std::uint8_t{0});
    CHECK_CONSTEXPR_EQ(kCharClass[200], std::uint8_t{0});
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
