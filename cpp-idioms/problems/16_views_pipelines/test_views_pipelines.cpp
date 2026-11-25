#include "views_pipelines.hpp"

#include <map>
#include <string>
#include <vector>

#include "testing.hpp"

using namespace idioms;

TEST(symbols_traded_values) {
    std::vector<Trade> t{{"AAPL", 100}, {"MSFT", 5}, {"GOOG", 50}, {"AAPL", 49}};
    CHECK(symbols_traded(t, 50) == (std::vector<std::string>{"AAPL", "GOOG"}));
    CHECK(symbols_traded(t, 1000).empty());
}

TEST(split_fields_values) {
    std::string line = "35=D,55=AAPL,,38=100";
    auto f = split_fields(line, ',');
    CHECK_EQ(f.size(), std::size_t{4});
    CHECK_EQ(f[0], std::string_view("35=D"));
    CHECK_EQ(f[2], std::string_view(""));
    CHECK_EQ(f[3], std::string_view("38=100"));
    CHECK(f[1].data() == line.data() + 5);  // views into the original, no copies
    CHECK_EQ(split_fields("solo", ',').size(), std::size_t{1});
}

TEST(last_even_values) {
    CHECK(last_even({1, 2, 3, 4, 5, 6, 7, 8, 9}, 3) == (std::vector<int>{8, 6, 4}));
    CHECK(last_even({2, 4}, 10) == (std::vector<int>{4, 2}));
    CHECK(last_even({1, 3}, 2).empty());
}

TEST(returns_values) {
    auto r = returns({100.0, 110.0, 99.0});
    CHECK_EQ(r.size(), std::size_t{2});
    CHECK_NEAR(r[0], 0.10, 1e-12);
    CHECK_NEAR(r[1], -0.10, 1e-12);
    CHECK(returns({}).empty());
    CHECK(returns({5.0}).empty());
}

TEST(total_position_values) {
    std::map<std::string, long> book{{"AAPL", 100}, {"MSFT", -40}, {"GOOG", 15}};
    CHECK_EQ(total_position(book), 75L);
    CHECK_EQ(total_position({}), 0L);
}

TEST(all_fill_ids_values) {
    CHECK(all_fill_ids({{1, 2}, {}, {3}, {4, 5, 6}}) == (std::vector<int>{1, 2, 3, 4, 5, 6}));
    CHECK(all_fill_ids({}).empty());
}

TEST(first_after_open_values) {
    std::vector<Event> ev{{1, 9.0}, {5, 10.0}, {6, 10.5}, {7, 0.0}, {8, 11.0}};
    CHECK(first_after_open(ev, 5) == (std::vector<double>{10.0, 10.5}));
    CHECK(first_after_open(ev, 8) == (std::vector<double>{11.0}));
    CHECK(first_after_open(ev, 100).empty());
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
