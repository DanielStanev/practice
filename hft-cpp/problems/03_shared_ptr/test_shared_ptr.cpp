#include "shared_ptr.hpp"

#include <atomic>
#include <cstdlib>
#include <new>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include "alloc_counter.hpp"
#include "testing.hpp"

using hft::SharedPtr;
using hft::WeakPtr;

namespace {

struct Tracked {
    static inline std::atomic<int> alive{0};
    static inline std::atomic<int> destroyed{0};
    int value;
    explicit Tracked(int v = 0) : value(v) { ++alive; }
    ~Tracked() {
        --alive;
        ++destroyed;
    }
    static void reset() {
        alive = 0;
        destroyed = 0;
    }
};

}  // namespace

TEST(empty_pointer) {
    SharedPtr<int> p;
    CHECK(p.get() == nullptr);
    CHECK_FALSE(static_cast<bool>(p));
    CHECK_EQ(p.use_count(), 0);
}

TEST(owns_raw_pointer) {
    Tracked::reset();
    {
        SharedPtr<Tracked> p(new Tracked(5));
        CHECK_EQ(p->value, 5);
        CHECK_EQ(p.use_count(), 1);
    }
    CHECK_EQ(Tracked::alive.load(), 0);
    CHECK_EQ(Tracked::destroyed.load(), 1);
}

TEST(copies_share_ownership) {
    Tracked::reset();
    SharedPtr<Tracked> a(new Tracked(1));
    {
        SharedPtr<Tracked> b = a;
        SharedPtr<Tracked> c;
        c = b;
        CHECK_EQ(a.use_count(), 3);
        CHECK(c.get() == a.get());
    }
    CHECK_EQ(a.use_count(), 1);
    CHECK_EQ(Tracked::alive.load(), 1);
    a.reset();
    CHECK_EQ(Tracked::alive.load(), 0);
}

TEST(moves_do_not_touch_count) {
    SharedPtr<Tracked> a(new Tracked(1));
    SharedPtr<Tracked> b(std::move(a));
    CHECK(a.get() == nullptr);
    CHECK_EQ(b.use_count(), 1);
    SharedPtr<Tracked> c;
    c = std::move(b);
    CHECK_EQ(c.use_count(), 1);
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<SharedPtr<int>>);
}

TEST(self_assignment) {
    Tracked::reset();
    SharedPtr<Tracked> a(new Tracked(1));
    SharedPtr<Tracked>& alias = a;
    a = alias;
    CHECK_EQ(a.use_count(), 1);
    a = std::move(alias);
    CHECK(a.get() != nullptr);
    CHECK_EQ(Tracked::alive.load(), 1);
}

TEST(reset_with_new_pointer) {
    Tracked::reset();
    SharedPtr<Tracked> a(new Tracked(1));
    SharedPtr<Tracked> b = a;
    a.reset(new Tracked(2));
    CHECK_EQ(a->value, 2);
    CHECK_EQ(b->value, 1);
    CHECK_EQ(a.use_count(), 1);
    CHECK_EQ(b.use_count(), 1);
}

TEST(make_shared_is_one_allocation) {
    long before = alloc_counter::count();
    auto p = hft::make_shared<std::pair<long, long>>(1, 2);
    long after = alloc_counter::count();
    CHECK_EQ(after - before, 1L);
    CHECK_EQ(p->second, 2L);

    before = alloc_counter::count();
    SharedPtr<int> q(new int(3));
    after = alloc_counter::count();
    CHECK_EQ(after - before, 2L);  // the int and the control block
}

TEST(make_shared_forwards) {
    auto s = hft::make_shared<std::string>(3, 'z');
    CHECK_EQ(*s, std::string("zzz"));
}

TEST(weak_does_not_keep_alive) {
    Tracked::reset();
    WeakPtr<Tracked> w;
    {
        auto s = hft::make_shared<Tracked>(9);
        w = s;
        CHECK_EQ(w.use_count(), 1);
        CHECK_FALSE(w.expired());
        auto locked = w.lock();
        CHECK_EQ(locked->value, 9);
        CHECK_EQ(s.use_count(), 2);
    }
    // Object is gone, but the control block must still be valid for w.
    CHECK_EQ(Tracked::alive.load(), 0);
    CHECK(w.expired());
    CHECK(w.lock().get() == nullptr);
    CHECK_EQ(w.use_count(), 0);
}

TEST(weak_copies_and_moves) {
    auto s = hft::make_shared<Tracked>(1);
    WeakPtr<Tracked> a(s);
    WeakPtr<Tracked> b(a);
    WeakPtr<Tracked> c(std::move(a));
    CHECK(c.lock().get() == s.get());
    CHECK(b.lock().get() == s.get());
    s.reset();
    CHECK(b.expired());
    CHECK(c.expired());
}

TEST(control_block_freed_with_last_weak) {
    // ASan will flag a leak (when all tests pass) or a use-after-free if the
    // control block lifetime is wrong.
    WeakPtr<Tracked> w;
    {
        SharedPtr<Tracked> s(new Tracked(1));
        w = s;
    }
    WeakPtr<Tracked> w2 = w;
    w.reset();
    CHECK(w2.expired());
}

TEST(concurrent_copies) {
    Tracked::reset();
    {
        auto shared = hft::make_shared<Tracked>(7);
        std::vector<std::thread> threads;
        for (int t = 0; t < 8; ++t) {
            threads.emplace_back([shared] {
                for (int i = 0; i < 20000; ++i) {
                    SharedPtr<Tracked> copy = shared;
                    SharedPtr<Tracked> moved = std::move(copy);
                    if (moved->value != 7) std::abort();
                }
            });
        }
        for (auto& t : threads) t.join();
        CHECK_EQ(shared.use_count(), 1);
    }
    CHECK_EQ(Tracked::destroyed.load(), 1);
}

TEST(concurrent_lock_vs_last_release) {
    // lock() racing with the last owner going away must either succeed
    // (and keep the object alive) or return empty. The object must be
    // destroyed exactly once.
    for (int round = 0; round < 200; ++round) {
        Tracked::reset();
        auto owner = hft::make_shared<Tracked>(round);
        WeakPtr<Tracked> weak(owner);
        std::atomic<bool> go{false};
        std::thread locker([&] {
            while (!go.load()) {
            }
            for (int i = 0; i < 100; ++i) {
                if (auto s = weak.lock()) {
                    if (s->value != round) std::abort();
                }
            }
        });
        go.store(true);
        owner.reset();
        locker.join();
        CHECK_EQ(Tracked::destroyed.load(), 1);
        CHECK(weak.expired());
    }
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
