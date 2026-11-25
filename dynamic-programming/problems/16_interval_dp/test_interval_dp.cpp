#include "interval_dp.hpp"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "testing.hpp"

namespace {

// Parses a fully parenthesized chain and returns its cost; checks it uses A.. in order.
struct ChainParser {
    const std::string& s;
    const std::vector<int>& dims;
    std::size_t pos = 0;
    int next_matrix = 0;
    std::int64_t cost = 0;
    bool ok = true;
    // returns {rows, cols}
    std::pair<int, int> parse() {
        if (pos >= s.size()) return ok = false, std::pair{0, 0};
        if (s[pos] == '(') {
            ++pos;
            auto l = parse();
            auto r = parse();
            if (pos >= s.size() || s[pos] != ')') ok = false;
            ++pos;
            if (l.second != r.first) ok = false;
            cost += static_cast<std::int64_t>(l.first) * l.second * r.second;
            return {l.first, r.second};
        }
        char expect = static_cast<char>('A' + next_matrix);
        if (s[pos] != expect) ok = false;
        ++pos;
        ++next_matrix;
        return {dims[next_matrix - 1], dims[next_matrix]};
    }
};

std::int64_t brute_chain(const std::vector<int>& d, int i, int j) {
    if (i == j) return 0;
    std::int64_t best = INT64_MAX;
    for (int k = i; k < j; ++k)
        best = std::min(best, brute_chain(d, i, k) + brute_chain(d, k + 1, j) +
                                  static_cast<std::int64_t>(d[i - 1]) * d[k] * d[j]);
    return best;
}

// Try every burst order.
std::int64_t brute_balloons(std::vector<int> v) {
    if (v.empty()) return 0;
    std::int64_t best = 0;
    for (std::size_t i = 0; i < v.size(); ++i) {
        std::int64_t l = i > 0 ? v[i - 1] : 1, r = i + 1 < v.size() ? v[i + 1] : 1;
        auto w = v;
        w.erase(w.begin() + static_cast<long>(i));
        best = std::max(best, l * v[i] * r + brute_balloons(w));
    }
    return best;
}

// Try every cut order.
std::int64_t brute_cuts(std::int64_t length, std::vector<std::int64_t> cuts) {
    std::sort(cuts.begin(), cuts.end());
    std::int64_t best = INT64_MAX;
    do {
        std::vector<std::int64_t> made{0, length};
        std::int64_t cost = 0;
        for (auto c : cuts) {
            auto it = std::upper_bound(made.begin(), made.end(), c);
            cost += *it - *(it - 1);
            made.insert(it, c);
        }
        best = std::min(best, cost);
    } while (std::next_permutation(cuts.begin(), cuts.end()));
    return best == INT64_MAX ? 0 : best;
}

}  // namespace

TEST(chain_examples) {
    CHECK_EQ(dp::matrix_chain_cost({10, 30, 5, 60}), 4500);
    CHECK_EQ(dp::matrix_chain_cost({40, 20, 30, 10, 30}), 26000);
    CHECK_EQ(dp::matrix_chain_cost({10, 20}), 0);
    CHECK_EQ(dp::matrix_chain_cost({30, 35, 15, 5, 10, 20, 25}), 15125);  // CLRS
}

TEST(chain_matches_brute_force) {
    std::mt19937_64 rng(44);
    for (int iter = 0; iter < 300; ++iter) {
        std::vector<int> d(2 + rng() % 8);
        for (auto& x : d) x = 1 + static_cast<int>(rng() % 20);
        CHECK_EQ(dp::matrix_chain_cost(d), brute_chain(d, 1, static_cast<int>(d.size()) - 1));
    }
}

TEST(chain_large) {
    std::vector<int> d(301);
    for (int i = 0; i <= 300; ++i) d[i] = 1 + (i * 37) % 100;
    std::int64_t c = dp::matrix_chain_cost(d);
    CHECK_EQ(c, 665102);
}

TEST(order_examples) {
    CHECK_EQ(dp::matrix_chain_order({10, 30, 5, 60}), std::string("((AB)C)"));
    CHECK_EQ(dp::matrix_chain_order({10, 20}), std::string("A"));
    std::vector<int> clrs{30, 35, 15, 5, 10, 20, 25};
    auto s = dp::matrix_chain_order(clrs);  // CLRS gives ((A(BC))((DE)F))
    ChainParser p{s, clrs};
    p.parse();
    CHECK(p.ok);
    CHECK_EQ(p.cost, 15125);
}

TEST(order_is_valid_and_optimal) {
    std::mt19937_64 rng(45);
    for (int iter = 0; iter < 300; ++iter) {
        std::vector<int> d(2 + rng() % 26);
        for (auto& x : d) x = 1 + static_cast<int>(rng() % 50);
        auto s = dp::matrix_chain_order(d);
        ChainParser p{s, d};
        p.parse();
        CHECK(p.ok);
        CHECK_EQ(p.pos, s.size());
        CHECK_EQ(p.next_matrix, static_cast<int>(d.size()) - 1);
        CHECK_EQ(p.cost, dp::matrix_chain_cost(d));
    }
}

TEST(balloons_examples) {
    CHECK_EQ(dp::burst_balloons({3, 1, 5, 8}), 167);
    CHECK_EQ(dp::burst_balloons({1, 5}), 10);
    CHECK_EQ(dp::burst_balloons({}), 0);
    CHECK_EQ(dp::burst_balloons({7}), 7);
}

TEST(balloons_match_brute_force) {
    std::mt19937_64 rng(46);
    for (int iter = 0; iter < 200; ++iter) {
        std::vector<int> v(rng() % 7);
        for (auto& x : v) x = static_cast<int>(rng() % 10);
        CHECK_EQ(dp::burst_balloons(v), brute_balloons(v));
    }
}

TEST(balloons_large) {
    std::vector<int> v(300, 100);
    CHECK_EQ(dp::burst_balloons(v), 298010100);
}

TEST(cuts_examples) {
    CHECK_EQ(dp::min_cut_cost(7, {1, 3, 4, 5}), 16);
    CHECK_EQ(dp::min_cut_cost(9, {5, 6, 1, 4, 2}), 22);
    CHECK_EQ(dp::min_cut_cost(10, {}), 0);
    CHECK_EQ(dp::min_cut_cost(10, {5}), 10);
}

TEST(cuts_match_brute_force) {
    std::mt19937_64 rng(47);
    for (int iter = 0; iter < 200; ++iter) {
        std::int64_t len = 2 + static_cast<std::int64_t>(rng() % 30);
        std::vector<std::int64_t> all(len - 1);
        std::iota(all.begin(), all.end(), 1);
        std::shuffle(all.begin(), all.end(), rng);
        all.resize(std::min<std::size_t>(all.size(), rng() % 7));
        CHECK_EQ(dp::min_cut_cost(len, all), brute_cuts(len, all));
    }
}

TEST(cuts_large) {
    std::vector<std::int64_t> cuts;
    for (int i = 1; i <= 300; ++i) cuts.push_back(static_cast<std::int64_t>(i) * 3'000'000);
    CHECK_EQ(dp::min_cut_cost(1'000'000'000, cuts), 7956000000LL);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
