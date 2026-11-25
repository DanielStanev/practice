#include "slot_map.hpp"

#include <algorithm>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "testing.hpp"

using idioms::Handle;
using idioms::SlotMap;

TEST(insert_get_erase) {
    SlotMap<std::string> m;
    Handle a = m.insert("alpha");
    Handle b = m.insert("bravo");
    Handle c = m.insert("charlie");
    CHECK_EQ(m.size(), std::size_t{3});
    CHECK_EQ(*m.get(b), std::string("bravo"));
    CHECK(m.erase(a));
    CHECK_FALSE(m.erase(a));
    CHECK(m.get(a) == nullptr);
    CHECK_EQ(*m.get(b), std::string("bravo"));  // survived the swap-and-pop
    CHECK_EQ(*m.get(c), std::string("charlie"));
    CHECK_EQ(m.size(), std::size_t{2});
}

TEST(default_and_garbage_handles_are_invalid) {
    SlotMap<int> m;
    CHECK(m.get(Handle{}) == nullptr);
    m.insert(1);
    CHECK(m.get(Handle{}) == nullptr);
    CHECK(m.get(Handle{12345, 1}) == nullptr);
    CHECK_FALSE(m.erase(Handle{99, 1}));
}

TEST(slot_reuse_keeps_old_handles_stale) {
    SlotMap<int> m;
    Handle a = m.insert(1);
    m.erase(a);
    Handle b = m.insert(2);
    CHECK_EQ(b.index, a.index);  // slot reused
    CHECK(!(a == b));            // but a new generation
    CHECK(m.get(a) == nullptr);
    CHECK_EQ(*m.get(b), 2);
    CHECK_EQ(m.slot_count(), std::size_t{1});
}

TEST(handles_survive_reallocation) {
    SlotMap<std::unique_ptr<int>> m;  // move-only payload
    Handle first = m.insert(std::make_unique<int>(42));
    std::vector<Handle> hs;
    for (int i = 0; i < 10000; ++i) hs.push_back(m.insert(std::make_unique<int>(i)));
    for (int i = 0; i < 10000; i += 3) m.erase(hs[i]);
    CHECK_EQ(**m.get(first), 42);
    for (int i = 0; i < 10000; ++i) {
        auto* p = m.get(hs[i]);
        if (i % 3 == 0) CHECK(p == nullptr);
        else CHECK_EQ(**p, i);
    }
}

TEST(dense_iteration) {
    SlotMap<int> m;
    std::vector<Handle> hs;
    for (int i = 0; i < 10; ++i) hs.push_back(m.insert(i));
    m.erase(hs[2]);
    m.erase(hs[7]);
    std::vector<int> seen(m.begin(), m.end());
    std::sort(seen.begin(), seen.end());
    CHECK(seen == (std::vector<int>{0, 1, 3, 4, 5, 6, 8, 9}));
    for (int& v : m) v *= 10;
    CHECK_EQ(*m.get(hs[9]), 90);
    CHECK_TRAIT(std::contiguous_iterator<decltype(m.begin())>);
}

TEST(erase_if_visits_everything) {
    SlotMap<int> m;
    std::vector<Handle> hs;
    for (int i = 0; i < 100; ++i) hs.push_back(m.insert(i));
    // Consecutive matches at the end are the trap: the swapped-in value must be tested too.
    CHECK_EQ(m.erase_if([](int v) { return v % 2 == 0 || v > 90; }), std::size_t{55});
    CHECK_EQ(m.size(), std::size_t{45});
    for (int v : m) CHECK(v % 2 == 1 && v <= 90);
    for (int i = 0; i < 100; ++i) CHECK_EQ(m.contains(hs[i]), i % 2 == 1 && i <= 90);
}

TEST(churn_matches_model) {
    std::mt19937 rng(11);
    SlotMap<long> m;
    std::vector<std::pair<Handle, long>> live;
    std::vector<Handle> dead;
    for (int step = 0; step < 50000; ++step) {
        if (live.empty() || rng() % 3 != 0) {
            long v = step;
            live.push_back({m.insert(v), v});
        } else {
            std::size_t i = rng() % live.size();
            CHECK(m.erase(live[i].first));
            dead.push_back(live[i].first);
            live[i] = live.back();
            live.pop_back();
        }
    }
    CHECK_EQ(m.size(), live.size());
    for (auto& [h, v] : live) CHECK_EQ(*m.get(h), v);
    for (auto& h : dead) CHECK(m.get(h) == nullptr);
    CHECK(m.slot_count() <= live.size() + 2000);  // slots were recycled
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
