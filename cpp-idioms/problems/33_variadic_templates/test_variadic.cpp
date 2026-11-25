#include "variadic.hpp"

#include <array>
#include <string>
#include <tuple>
#include <type_traits>

#include "testing.hpp"

using namespace idioms;

TEST(sum_fold) {
    CHECK_EQ(sum(1, 2, 3, 4), 10);
    CHECK_EQ(sum(5), 5);
    CHECK_EQ(sum(1.5, 2, 3L), 6.5);
    CHECK_EQ(sum(std::string("a"), "b", "c"), std::string("abc"));
}

TEST(all_same) {
    CHECK_TRAIT(all_same_v<>);
    CHECK_TRAIT(all_same_v<int>);
    CHECK_TRAIT(all_same_v<int, int, int>);
    CHECK_TRAIT(!all_same_v<int, long>);
    CHECK_TRAIT(!all_same_v<int, int, const int>);
}

TEST(join_args_values) {
    CHECK_EQ(join_args(", ", 1, "two", 3.5), std::string("1, two, 3.5"));
    CHECK_EQ(join_args("|", 'x'), std::string("x"));
    CHECK_EQ(join_args("|"), std::string(""));
}

TEST(for_each_in_tuple_order_and_mutation) {
    std::tuple<int, std::string, double> t{1, "a", 2.5};
    std::string seen;
    for_each_in_tuple(t, [&](const auto& x) {
        if constexpr (std::is_same_v<std::decay_t<decltype(x)>, std::string>) seen += x;
        else seen += std::to_string(static_cast<int>(x));
        seen += ";";
    });
    CHECK_EQ(seen, std::string("1;a;2;"));
    for_each_in_tuple(t, [](auto& x) { x = x + x; });
    CHECK_EQ(std::get<0>(t), 2);
    CHECK_EQ(std::get<1>(t), std::string("aa"));
    CHECK_EQ(std::get<2>(t), 5.0);
}

TEST(transform_tuple_types) {
    auto r = transform_tuple(std::tuple{1, 2.5, std::string("abc")}, [](const auto& x) {
        if constexpr (std::is_same_v<std::decay_t<decltype(x)>, std::string>) return x.size();
        else return x * 2;
    });
    CHECK_TRAIT(std::is_same_v<decltype(r), std::tuple<int, double, std::size_t>>);
    CHECK_EQ(std::get<0>(r), 2);
    CHECK_EQ(std::get<1>(r), 5.0);
    CHECK_EQ(std::get<2>(r), std::size_t{3});
}

TEST(make_table_constexpr) {
    constexpr auto squares = make_table<6>([](std::size_t i) { return i * i; });
    static_assert(squares.size() == 6);
    CHECK_EQ(squares[5], std::size_t{25});
    CHECK_EQ(squares[0], std::size_t{0});
    constexpr auto pow10 = make_table<5>([](std::size_t i) {
        long v = 1;
        for (std::size_t k = 0; k < i; ++k) v *= 10;
        return v;
    });
    CHECK_EQ(pow10[4], 10000L);
}

TEST(type_list) {
    using L = TypeList<int, double, char, double>;
    CHECK_EQ(list_size_v<L>, std::size_t{4});
    CHECK_EQ(list_size_v<TypeList<>>, std::size_t{0});
    CHECK_TRAIT(contains_v<char, L>);
    CHECK_TRAIT(!contains_v<long, L>);
    CHECK_TRAIT(!contains_v<int, TypeList<>>);
    CHECK_EQ((index_of_v<int, L>), std::size_t{0});
    CHECK_EQ((index_of_v<double, L>), std::size_t{1});  // first occurrence
    CHECK_EQ((index_of_v<char, L>), std::size_t{2});
    CHECK_TRAIT(std::is_same_v<at_t<0, L>, int>);
    CHECK_TRAIT(std::is_same_v<at_t<2, L>, char>);
    CHECK_TRAIT(std::is_same_v<at_t<3, L>, double>);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
