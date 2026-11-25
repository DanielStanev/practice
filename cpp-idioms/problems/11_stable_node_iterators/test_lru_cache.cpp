#include "lru_cache.hpp"

#include <algorithm>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "alloc_counter.hpp"
#include "testing.hpp"

using idioms::LruCache;

TEST(basic_lru_order) {
    LruCache<int, long> c(3);
    c.put(1, 10);
    c.put(2, 20);
    c.put(3, 30);
    CHECK(c.keys_mru_order() == (std::vector<int>{3, 2, 1}));
    CHECK(c.get(1) != nullptr);
    CHECK_EQ(*c.get(1), 10L);
    CHECK(c.keys_mru_order() == (std::vector<int>{1, 3, 2}));
    c.put(4, 40);  // evicts 2
    CHECK(c.get(2) == nullptr);
    CHECK_EQ(c.evictions(), std::size_t{1});
    CHECK(c.keys_mru_order() == (std::vector<int>{4, 1, 3}));
    c.put(3, 33);  // update moves to front, no eviction
    CHECK(c.keys_mru_order() == (std::vector<int>{3, 4, 1}));
    CHECK_EQ(*c.get(3), 33L);
    CHECK_EQ(c.size(), std::size_t{3});
    CHECK_EQ(c.evictions(), std::size_t{1});
}

TEST(capacity_one) {
    LruCache<std::string, std::string> c(1);
    c.put("a", "x");
    c.put("b", "y");
    CHECK(c.get("a") == nullptr);
    CHECK_EQ(*c.get("b"), std::string("y"));
    CHECK_EQ(c.size(), std::size_t{1});
}

TEST(pointers_are_stable) {
    LruCache<int, long> c(4);
    for (int i = 0; i < 4; ++i) c.put(i, i * 100);
    long* p = c.get(2);
    CHECK(p != nullptr);
    c.get(0);
    c.get(3);
    c.put(1, 111);
    c.get(2);
    CHECK_EQ(*p, 200L);  // the node never moved in memory
    *p = 7;
    CHECK_EQ(*c.get(2), 7L);
}

TEST(no_allocation_in_steady_state) {
    LruCache<int, long> c(64);
    for (int i = 0; i < 64; ++i) c.put(i, i);
    long before = alloc_counter::count();
    for (int i = 0; i < 10000; ++i) {
        c.get(i % 97);           // hits and misses
        c.put(i % 50, i);        // sometimes existing, sometimes new (evicting)
        c.put(1000 + i, i);      // always new: eviction
    }
    CHECK_EQ(alloc_counter::count(), before);
    CHECK_EQ(c.size(), std::size_t{64});
}

TEST(matches_reference_model) {
    // A slow but obviously correct model: vector in MRU order.
    std::mt19937 rng(3);
    LruCache<int, long> c(16);
    std::vector<std::pair<int, long>> model;
    for (int step = 0; step < 20000; ++step) {
        int k = static_cast<int>(rng() % 40);
        auto pos = std::find_if(model.begin(), model.end(), [k](auto& e) { return e.first == k; });
        if (rng() % 2) {
            long* got = c.get(k);
            if (pos == model.end()) {
                CHECK(got == nullptr);
            } else {
                CHECK(got != nullptr);
                CHECK_EQ(*got, pos->second);
                auto e = *pos;
                model.erase(pos);
                model.insert(model.begin(), e);
            }
        } else {
            long v = step;
            c.put(k, v);
            if (pos != model.end()) model.erase(pos);
            model.insert(model.begin(), {k, v});
            if (model.size() > 16) model.pop_back();
        }
    }
    std::vector<int> keys;
    for (auto& e : model) keys.push_back(e.first);
    CHECK(c.keys_mru_order() == keys);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
