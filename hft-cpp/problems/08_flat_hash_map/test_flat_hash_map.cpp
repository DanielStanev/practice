#include "flat_hash_map.hpp"

#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>

#include "testing.hpp"

using hft::FlatHashMap;

namespace {

// Every key collides: exercises probing and deletion in the worst case.
struct ConstantHash {
    std::size_t operator()(std::uint64_t) const { return 42; }
};

// Only a few distinct hash values: long, interleaved probe runs.
struct CoarseHash {
    std::size_t operator()(std::uint64_t k) const { return k % 7; }
};

struct Tracked {
    static inline int alive = 0;
    int v = 0;
    Tracked() { ++alive; }
    explicit Tracked(int x) : v(x) { ++alive; }
    Tracked(const Tracked& o) : v(o.v) { ++alive; }
    Tracked(Tracked&& o) noexcept : v(o.v) { ++alive; }
    Tracked& operator=(const Tracked&) = default;
    Tracked& operator=(Tracked&&) noexcept = default;
    ~Tracked() { --alive; }
};

bool is_pow2(std::size_t x) { return x && (x & (x - 1)) == 0; }

}  // namespace

TEST(insert_find_erase_basic) {
    FlatHashMap<std::uint64_t, int> m;
    CHECK(m.empty());
    CHECK(m.insert(1, 10));
    CHECK(m.insert(2, 20));
    CHECK_FALSE(m.insert(1, 99));  // already present: no change
    CHECK_EQ(m.size(), 2u);
    CHECK(m.find(1) != nullptr);
    CHECK_EQ(*m.find(1), 10);
    CHECK(m.find(3) == nullptr);
    CHECK(m.contains(2));
    CHECK(m.erase(1));
    CHECK_FALSE(m.erase(1));
    CHECK_FALSE(m.contains(1));
    CHECK_EQ(m.size(), 1u);
}

TEST(insert_or_assign_and_subscript) {
    FlatHashMap<std::uint64_t, int> m;
    m.insert_or_assign(5, 1);
    m.insert_or_assign(5, 2);
    CHECK_EQ(*m.find(5), 2);
    CHECK_EQ(m.size(), 1u);
    m[7] += 3;  // default-constructs then adds
    m[7] += 3;
    CHECK_EQ(m[7], 6);
    CHECK_EQ(m.size(), 2u);
}

TEST(capacity_is_power_of_two_and_load_bounded) {
    FlatHashMap<std::uint64_t, int> m(10);
    CHECK(is_pow2(m.capacity()));
    CHECK(m.capacity() >= 10u);
    for (std::uint64_t i = 0; i < 10000; ++i) {
        m.insert(i, 0);
        CHECK(is_pow2(m.capacity()));
        CHECK(m.load_factor() <= 0.875);
    }
}

TEST(reserve_prevents_rehash) {
    FlatHashMap<std::uint64_t, int> m;
    m.reserve(5000);
    std::size_t cap = m.capacity();
    for (std::uint64_t i = 0; i < 5000; ++i) m.insert(i * 7919, int(i));
    CHECK_EQ(m.capacity(), cap);
    for (std::uint64_t i = 0; i < 5000; ++i) CHECK_EQ(*m.find(i * 7919), int(i));
}

TEST(all_keys_collide) {
    FlatHashMap<std::uint64_t, int, ConstantHash> m;
    for (std::uint64_t i = 0; i < 300; ++i) CHECK(m.insert(i, int(i)));
    for (std::uint64_t i = 0; i < 300; i += 2) CHECK(m.erase(i));
    for (std::uint64_t i = 0; i < 300; ++i) {
        if (i % 2 == 0) {
            CHECK(m.find(i) == nullptr);
        } else {
            CHECK(m.find(i) != nullptr);
            CHECK_EQ(*m.find(i), int(i));
        }
    }
    CHECK_EQ(m.size(), 150u);
}

TEST(erase_keeps_probe_chains_intact) {
    // With a coarse hash, runs from different home buckets interleave and
    // wrap around the end of the table. Deleting from the middle of a run
    // must not hide later keys.
    std::mt19937_64 rng(3);
    for (int round = 0; round < 50; ++round) {
        FlatHashMap<std::uint64_t, std::uint64_t, CoarseHash> m(8);
        std::unordered_map<std::uint64_t, std::uint64_t> ref;
        for (int i = 0; i < 2000; ++i) {
            std::uint64_t k = rng() % 64;
            if (rng() % 3 == 0) {
                CHECK_EQ(m.erase(k), ref.erase(k) == 1);
            } else {
                m.insert_or_assign(k, k * 3);
                ref[k] = k * 3;
            }
            CHECK_EQ(m.size(), ref.size());
        }
        for (std::uint64_t k = 0; k < 64; ++k) {
            auto* v = m.find(k);
            CHECK_EQ(v != nullptr, ref.count(k) == 1);
            if (v) CHECK_EQ(*v, ref[k]);
        }
    }
}

TEST(no_tombstone_buildup) {
    // A fixed working set churned many times must not grow the table:
    // deletions have to free slots for real (backward shift), not leave
    // tombstones that eventually force rehashes or endless probing.
    FlatHashMap<std::uint64_t, int> m;
    m.reserve(1000);
    std::size_t cap = m.capacity();
    std::uint64_t next = 0;
    for (std::uint64_t i = 0; i < 1000; ++i) m.insert(next++, 0);
    for (int i = 0; i < 200000; ++i) {
        CHECK(m.erase(next - 1000));
        m.insert(next++, 0);
    }
    CHECK_EQ(m.capacity(), cap);
    CHECK_EQ(m.size(), 1000u);
    CHECK(m.find(0) == nullptr);
}

TEST(random_ops_match_unordered_map) {
    FlatHashMap<std::uint64_t, std::uint64_t> m;
    std::unordered_map<std::uint64_t, std::uint64_t> ref;
    std::mt19937_64 rng(11);
    for (int i = 0; i < 300000; ++i) {
        std::uint64_t k = rng() % 5000;
        switch (rng() % 4) {
            case 0:
                CHECK_EQ(m.insert(k, i), ref.emplace(k, i).second);
                break;
            case 1:
                m.insert_or_assign(k, i);
                ref[k] = i;
                break;
            case 2:
                CHECK_EQ(m.erase(k), ref.erase(k) == 1);
                break;
            case 3: {
                auto* v = m.find(k);
                auto it = ref.find(k);
                CHECK_EQ(v != nullptr, it != ref.end());
                if (v) CHECK_EQ(*v, it->second);
                break;
            }
        }
    }
    CHECK_EQ(m.size(), ref.size());
    std::size_t visited = 0;
    m.for_each([&](const std::uint64_t& k, const std::uint64_t& v) {
        ++visited;
        CHECK_EQ(ref.at(k), v);
    });
    CHECK_EQ(visited, ref.size());
}

TEST(clustered_keys_still_fast) {
    // Order ids are often sequential or strided. With std::hash<uint64_t>
    // being the identity, `key & mask` would put all multiples of 4096 in
    // one bucket. This finishes instantly only if you mix the hash bits.
    FlatHashMap<std::uint64_t, int> m;
    for (std::uint64_t i = 0; i < 200000; ++i) m.insert(i << 12, 1);
    for (std::uint64_t i = 0; i < 200000; ++i) CHECK(m.contains(i << 12));
}

TEST(string_keys_and_values) {
    FlatHashMap<std::string, std::string> m;
    for (int i = 0; i < 1000; ++i) m.insert("key" + std::to_string(i), std::string(50, char('a' + i % 26)));
    for (int i = 0; i < 1000; i += 3) CHECK(m.erase("key" + std::to_string(i)));
    for (int i = 0; i < 1000; ++i) CHECK_EQ(m.contains("key" + std::to_string(i)), i % 3 != 0);
    CHECK_EQ(*m.find("key1"), std::string(50, 'b'));
}

TEST(values_destroyed_exactly_once) {
    Tracked::alive = 0;
    {
        FlatHashMap<std::uint64_t, Tracked> m;
        for (std::uint64_t i = 0; i < 500; ++i) m.insert(i, Tracked(int(i)));
        CHECK_EQ(Tracked::alive, 500);
        for (std::uint64_t i = 0; i < 100; ++i) m.erase(i);
        CHECK_EQ(Tracked::alive, 400);
        m.clear();
        CHECK_EQ(Tracked::alive, 0);
        CHECK(m.empty());
        for (std::uint64_t i = 0; i < 50; ++i) m.insert(i, Tracked(1));
    }
    CHECK_EQ(Tracked::alive, 0);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
