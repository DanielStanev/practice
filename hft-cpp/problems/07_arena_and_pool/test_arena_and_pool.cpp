#include "arena_and_pool.hpp"

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "alloc_counter.hpp"
#include "testing.hpp"

using hft::Arena;
using hft::ArenaAllocator;
using hft::ObjectPool;


namespace {

bool aligned(const void* p, std::size_t a) { return reinterpret_cast<std::uintptr_t>(p) % a == 0; }

struct Order {
    static inline int alive = 0;
    std::uint64_t id;
    std::int64_t price;
    std::uint32_t qty;
    Order(std::uint64_t i, std::int64_t p, std::uint32_t q) : id(i), price(p), qty(q) { ++alive; }
    ~Order() { --alive; }
};

struct alignas(64) CacheAligned {
    char data[64];
};

struct ThrowsOnConstruct {
    explicit ThrowsOnConstruct(bool fail) {
        if (fail) throw std::runtime_error("ctor failed");
    }
};

struct Pod {
    int a;
    double b;
};

}  // namespace

// ------------------------------------------------------------------ Arena

TEST(arena_bumps_and_aligns) {
    Arena arena(1024);
    CHECK_EQ(arena.capacity(), 1024u);
    CHECK_EQ(arena.used(), 0u);
    void* a = arena.allocate(1, 1);
    void* b = arena.allocate(8, 8);
    void* c = arena.allocate(3, 1);
    void* d = arena.allocate(16, 16);
    void* e = arena.allocate(64, 64);
    CHECK(a && b && c && d && e);
    CHECK(aligned(b, 8));
    CHECK(aligned(d, 16));
    CHECK(aligned(e, 64));
    CHECK(static_cast<char*>(b) >= static_cast<char*>(a) + 1);
    CHECK(static_cast<char*>(c) >= static_cast<char*>(b) + 8);
    CHECK(arena.used() >= 1 + 8 + 3 + 16 + 64);
    CHECK(arena.used() <= 1024u);
}

TEST(arena_returns_null_when_full) {
    Arena arena(128);
    CHECK(arena.allocate(100, 1) != nullptr);
    std::size_t used = arena.used();
    CHECK(arena.allocate(64, 1) == nullptr);
    CHECK_EQ(arena.used(), used);  // a failed allocation changes nothing
    CHECK(arena.allocate(28, 1) != nullptr);
    CHECK(arena.allocate(1, 1) == nullptr);
}

TEST(arena_reset_reuses_memory) {
    Arena arena(256);
    void* first = arena.allocate(200, 8);
    arena.reset();
    CHECK_EQ(arena.used(), 0u);
    void* again = arena.allocate(200, 8);
    CHECK(first == again);
}

TEST(arena_create_constructs) {
    Arena arena(256);
    Pod* p = arena.create<Pod>(Pod{3, 4.5});
    CHECK(p != nullptr);
    CHECK_EQ(p->a, 3);
    CHECK(aligned(p, alignof(Pod)));
    CHECK(arena.owns(p));
    int x = 0;
    CHECK_FALSE(arena.owns(&x));
}

TEST(arena_does_not_allocate_after_construction) {
    Arena arena(1 << 16);
    long before = alloc_counter::count();
    for (int i = 0; i < 1000; ++i) arena.allocate(32, 8);
    CHECK_EQ(alloc_counter::count(), before);
}

TEST(arena_allocator_with_std_vector) {
    Arena arena(1 << 16);
    long before = alloc_counter::count();
    {
        std::vector<int, ArenaAllocator<int>> v{ArenaAllocator<int>(arena)};
        for (int i = 0; i < 1000; ++i) v.push_back(i);
        CHECK(arena.owns(v.data()));
        CHECK_EQ(v[999], 999);
    }
    CHECK_EQ(alloc_counter::count(), before);
    CHECK(arena.used() >= 1000 * sizeof(int));
}

TEST(arena_allocator_rebinds_and_compares) {
    Arena a(1024), b(1024);
    ArenaAllocator<int> ia(a);
    ArenaAllocator<double> da(ia);  // rebind conversion
    CHECK(da.arena() == &a);
    CHECK(ia == da);
    CHECK_FALSE(ia == ArenaAllocator<int>(b));
}

TEST(arena_allocator_throws_bad_alloc) {
    Arena arena(64);
    ArenaAllocator<std::uint64_t> alloc(arena);
    CHECK_THROWS_AS(alloc.allocate(100), std::bad_alloc);
}

// ------------------------------------------------------------- ObjectPool

TEST(pool_acquire_constructs_release_destroys) {
    Order::alive = 0;
    ObjectPool<Order> pool(4);
    CHECK_EQ(pool.capacity(), 4u);
    CHECK_EQ(pool.available(), 4u);
    Order* o = pool.acquire(1u, 100, 10u);
    CHECK(o != nullptr);
    CHECK_EQ(o->id, 1u);
    CHECK_EQ(Order::alive, 1);
    CHECK_EQ(pool.available(), 3u);
    pool.release(o);
    CHECK_EQ(Order::alive, 0);
    CHECK_EQ(pool.available(), 4u);
}

TEST(pool_exhaustion_returns_null) {
    ObjectPool<Order> pool(3);
    Order* a = pool.acquire(1u, 1, 1u);
    Order* b = pool.acquire(2u, 1, 1u);
    Order* c = pool.acquire(3u, 1, 1u);
    CHECK(a && b && c);
    CHECK(pool.acquire(4u, 1, 1u) == nullptr);
    CHECK_EQ(pool.available(), 0u);
    pool.release(b);
    Order* d = pool.acquire(5u, 1, 1u);
    CHECK(d != nullptr);
    pool.release(a);
    pool.release(c);
    pool.release(d);
}

TEST(pool_reuses_most_recent_slot) {
    // LIFO reuse keeps the hottest (most recently touched) memory in cache.
    ObjectPool<Order> pool(8);
    Order* a = pool.acquire(1u, 1, 1u);
    Order* b = pool.acquire(2u, 1, 1u);
    pool.release(a);
    Order* c = pool.acquire(3u, 1, 1u);
    CHECK(c == a);
    pool.release(b);
    pool.release(c);
}

TEST(pool_respects_alignment) {
    ObjectPool<CacheAligned> pool(10);
    std::vector<CacheAligned*> v;
    for (int i = 0; i < 10; ++i) {
        v.push_back(pool.acquire());
        CHECK(aligned(v.back(), 64));
    }
    for (auto* p : v) pool.release(p);
}

TEST(pool_slot_returned_if_constructor_throws) {
    ObjectPool<ThrowsOnConstruct> pool(1);
    CHECK_THROWS_AS(pool.acquire(true), std::runtime_error);
    CHECK_EQ(pool.available(), 1u);
    auto* ok = pool.acquire(false);
    CHECK(ok != nullptr);
    pool.release(ok);
}

TEST(pool_does_not_allocate_on_hot_path) {
    ObjectPool<Order> pool(1000);
    long before = alloc_counter::count();
    std::vector<Order*> live;
    live.reserve(1000);
    before = alloc_counter::count();
    for (int round = 0; round < 10; ++round) {
        for (int i = 0; i < 1000; ++i) live.push_back(pool.acquire(std::uint64_t(i), 1, 1u));
        for (auto* p : live) pool.release(p);
        live.clear();
    }
    CHECK_EQ(alloc_counter::count(), before);
}

TEST(pool_owns) {
    ObjectPool<Order> pool(2);
    Order* a = pool.acquire(1u, 1, 1u);
    Order outside(9u, 9, 9u);
    CHECK(pool.owns(a));
    CHECK_FALSE(pool.owns(&outside));
    pool.release(a);
}

TEST(pool_random_stress) {
    Order::alive = 0;
    ObjectPool<Order> pool(256);
    std::vector<Order*> live;
    std::set<Order*> unique;
    std::mt19937 rng(7);
    for (int step = 0; step < 200000; ++step) {
        bool do_acquire = live.empty() || (live.size() < 256 && rng() % 2 == 0);
        if (do_acquire) {
            Order* o = pool.acquire(std::uint64_t(step), step, 1u);
            CHECK(o != nullptr);
            CHECK(unique.insert(o).second);  // never hand out a live slot twice
            live.push_back(o);
        } else {
            std::size_t i = rng() % live.size();
            Order* o = live[i];
            CHECK_EQ(o->price, static_cast<std::int64_t>(o->id));  // not corrupted
            unique.erase(o);
            pool.release(o);
            live[i] = live.back();
            live.pop_back();
        }
        CHECK_EQ(pool.available(), 256u - live.size());
    }
    for (auto* o : live) pool.release(o);
    CHECK_EQ(Order::alive, 0);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
