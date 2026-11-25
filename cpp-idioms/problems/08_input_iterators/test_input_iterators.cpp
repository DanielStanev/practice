#include "input_iterators.hpp"

#include <algorithm>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
template <class R>
constexpr bool kInputRange = std::ranges::input_range<R>;
}

TEST(lines_is_input_not_forward) {
    CHECK_TRAIT(std::ranges::input_range<Lines>);
    CHECK_TRAIT(!std::ranges::forward_range<Lines>);
    CHECK_TRAIT(!std::is_copy_constructible_v<Lines>);
}

template <class L>
void lines_impl() {
    if constexpr (kInputRange<L>) {
        std::istringstream in("first\nsecond\n\nfourth");
        L lines(in);
        std::vector<std::string> got;
        for (const std::string& s : lines) got.push_back(s);
        CHECK(got == (std::vector<std::string>{"first", "second", "", "fourth"}));

        std::istringstream empty("");
        L none(empty);
        CHECK(none.begin() == none.end());
    } else {
        ::testing::fail(__FILE__, __LINE__, "Lines is not an input range yet");
    }
}
TEST(lines_values) { lines_impl<Lines>(); }

template <class L>
void lines_views_impl() {
    if constexpr (kInputRange<L>) {
        std::istringstream in("AAPL 10\nMSFT 20\nAAPL 30\nGOOG 40\nAAPL 50\n");
        L lines(in);
        std::vector<int> qty;
        for (int q : lines | std::views::filter([](const std::string& s) { return s.starts_with("AAPL"); }) |
                         std::views::transform([](const std::string& s) { return std::stoi(s.substr(5)); }) |
                         std::views::take(2))
            qty.push_back(q);
        CHECK(qty == (std::vector<int>{10, 30}));
    } else {
        ::testing::fail(__FILE__, __LINE__, "Lines is not an input range yet");
    }
}
TEST(lines_with_views) { lines_views_impl<Lines>(); }

TEST(count_errors_lines) {
    std::istringstream in("INFO start\nERROR disk\nWARN x\nERROR net\nERRORS\n");
    CHECK_EQ(count_errors(in), 3L);
}

template <class G>
constexpr bool kGenReady = std::ranges::input_range<G>;

template <class F>
void generator_impl(F f, int& calls) {
    using G = Generator<F>;
    if constexpr (kGenReady<G>) {
        {
            auto g = make_generator(f);
            CHECK_EQ(calls, 0);  // nothing happens until begin()
            std::vector<int> got;
            for (int x : g) got.push_back(x);
            CHECK(got == (std::vector<int>{1, 1, 2, 3, 5, 8, 13}));
            CHECK_EQ(calls, 8);  // 7 values + the final nullopt
        }
        calls = 0;
        {
            auto g = make_generator(f);
            std::vector<int> evens;
            for (int x : g | std::views::filter([](int v) { return v % 2 == 0; })) evens.push_back(x);
            CHECK(evens == (std::vector<int>{2, 8}));
            CHECK_EQ(calls, 8);  // filter dereferences twice; F must still be called once per step
        }
        calls = 0;
        {
            auto g = make_generator(f);
            auto it = g.begin();
            CHECK_EQ(*it, 1);
            CHECK_EQ(*it, 1);
            ++it;
            it++;
            CHECK_EQ(*it, 2);
            CHECK_EQ(calls, 3);
        }
    } else {
        ::testing::fail(__FILE__, __LINE__, "Generator is not an input range yet");
    }
}

TEST(generator_calls_once_per_step) {
    int calls = 0;
    auto fib = [&calls, a = 0, b = 1]() mutable -> std::optional<int> {
        ++calls;
        if (b > 13) return std::nullopt;
        int r = b;
        b = a + b;
        a = r;
        return r;
    };
    generator_impl(fib, calls);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
