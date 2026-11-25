#include "edit_distance.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <queue>
#include <random>
#include <string>

#include "testing.hpp"

using dp::Edit;
using dp::Op;

namespace {

// Applies an alignment script; returns false if it is malformed.
bool apply_script(const std::string& a, const std::vector<Edit>& script, std::string& out, int& cost) {
    out.clear();
    cost = 0;
    std::size_t i = 0;
    for (const auto& e : script) {
        switch (e.op) {
            case Op::Keep:
                if (i >= a.size()) return false;
                out += a[i++];
                break;
            case Op::Replace:
                if (i >= a.size()) return false;
                ++i;
                out += e.ch;
                ++cost;
                break;
            case Op::Delete:
                if (i >= a.size()) return false;
                ++i;
                ++cost;
                break;
            case Op::Insert:
                out += e.ch;
                ++cost;
                break;
        }
    }
    return i == a.size();
}

// Brute force: breadth-first search over strings (tiny alphabet and lengths).
int brute_distance(const std::string& a, const std::string& b) {
    const std::string alphabet = "ab";
    std::map<std::string, int> dist{{a, 0}};
    std::queue<std::string> q;
    q.push(a);
    const std::size_t limit = std::max(a.size(), b.size()) + 1;
    while (!q.empty()) {
        std::string s = q.front();
        q.pop();
        int d = dist[s];
        if (s == b) return d;
        std::vector<std::string> next;
        for (std::size_t i = 0; i < s.size(); ++i) {
            next.push_back(s.substr(0, i) + s.substr(i + 1));
            for (char c : alphabet) next.push_back(s.substr(0, i) + c + s.substr(i + 1));
        }
        if (s.size() < limit)
            for (std::size_t i = 0; i <= s.size(); ++i)
                for (char c : alphabet) next.push_back(s.substr(0, i) + c + s.substr(i));
        for (auto& t : next)
            if (!dist.count(t)) {
                dist[t] = d + 1;
                q.push(t);
            }
    }
    return -1;
}

std::string random_string(std::mt19937_64& rng, std::size_t n, int alphabet) {
    std::string s(n, 'a');
    for (auto& c : s) c = static_cast<char>('a' + rng() % alphabet);
    return s;
}

}  // namespace

TEST(distance_examples) {
    CHECK_EQ(dp::edit_distance("horse", "ros"), 3);
    CHECK_EQ(dp::edit_distance("intention", "execution"), 5);
    CHECK_EQ(dp::edit_distance("", ""), 0);
    CHECK_EQ(dp::edit_distance("", "abc"), 3);
    CHECK_EQ(dp::edit_distance("abc", ""), 3);
    CHECK_EQ(dp::edit_distance("kitten", "sitting"), 3);
    CHECK_EQ(dp::edit_distance("AAPL", "APPL"), 1);
}

TEST(distance_matches_brute_force) {
    std::mt19937_64 rng(33);
    for (int iter = 0; iter < 300; ++iter) {
        auto a = random_string(rng, rng() % 6, 2);
        auto b = random_string(rng, rng() % 6, 2);
        CHECK_EQ(dp::edit_distance(a, b), brute_distance(a, b));
    }
}

TEST(distance_large) {
    std::string a(5000, 'a'), b(4000, 'b');
    CHECK_EQ(dp::edit_distance(a, b), 5000);
    std::string c = a;
    for (int i = 0; i < 5000; i += 10) c[i] = 'z';
    CHECK_EQ(dp::edit_distance(a, c), 500);
    CHECK_EQ(dp::edit_distance(a.substr(1), a), 1);
}

TEST(script_examples) {
    for (auto [a, b] : std::vector<std::pair<std::string, std::string>>{
             {"horse", "ros"}, {"", "abc"}, {"abc", ""}, {"", ""}, {"same", "same"}, {"kitten", "sitting"}}) {
        auto script = dp::edit_script(a, b);
        std::string out;
        int cost = 0;
        CHECK(apply_script(a, script, out, cost));
        CHECK_EQ(out, b);
        CHECK_EQ(cost, dp::edit_distance(a, b));
    }
}

TEST(script_is_valid_and_minimal) {
    std::mt19937_64 rng(34);
    for (int iter = 0; iter < 500; ++iter) {
        auto a = random_string(rng, rng() % 15, 3);
        auto b = random_string(rng, rng() % 15, 3);
        auto script = dp::edit_script(a, b);
        std::string out;
        int cost = 0;
        CHECK(apply_script(a, script, out, cost));
        CHECK_EQ(out, b);
        CHECK_EQ(cost, dp::edit_distance(a, b));
    }
}

TEST(script_large) {
    std::mt19937_64 rng(35);
    auto a = random_string(rng, 2000, 4);
    auto b = random_string(rng, 1800, 4);
    auto script = dp::edit_script(a, b);
    std::string out;
    int cost = 0;
    CHECK(apply_script(a, script, out, cost));
    CHECK_EQ(out, b);
    CHECK_EQ(cost, dp::edit_distance(a, b));
}

TEST(weighted_examples) {
    CHECK_EQ(dp::edit_distance_weighted("horse", "ros", 1, 1, 1), 3);
    CHECK_EQ(dp::edit_distance_weighted("abc", "abd", 1, 1, 5), 2);  // delete + insert beats replace
    CHECK_EQ(dp::edit_distance_weighted("abc", "abd", 4, 4, 5), 5);
    CHECK_EQ(dp::edit_distance_weighted("", "xyz", 7, 1, 1), 21);
    CHECK_EQ(dp::edit_distance_weighted("xyz", "", 1, 7, 1), 21);
    CHECK_EQ(dp::edit_distance_weighted("abc", "xyz", 0, 0, 100), 0);
}

TEST(weighted_unit_costs_match_unweighted) {
    std::mt19937_64 rng(36);
    for (int iter = 0; iter < 300; ++iter) {
        auto a = random_string(rng, rng() % 30, 3);
        auto b = random_string(rng, rng() % 30, 3);
        CHECK_EQ(dp::edit_distance_weighted(a, b, 1, 1, 1), static_cast<std::int64_t>(dp::edit_distance(a, b)));
    }
}

TEST(weighted_large) {
    std::string a(5000, 'a'), b(5000, 'b');
    CHECK_EQ(dp::edit_distance_weighted(a, b, 1'000'000'000, 1'000'000'000, 1'000'000'000),
             5000LL * 1'000'000'000LL);
    CHECK_EQ(dp::edit_distance_weighted(a, b, 2, 3, 100), 5000LL * 5);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
