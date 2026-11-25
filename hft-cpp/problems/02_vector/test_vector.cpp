#include "vector.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "testing.hpp"

using hft::Vector;

namespace {

// Counts live objects and how they were made.
struct Tracked {
    static inline int alive = 0;
    static inline int copies = 0;
    static inline int moves = 0;
    static inline int default_ctors = 0;
    int value = 0;

    Tracked() : value(-1) {
        ++alive;
        ++default_ctors;
    }
    explicit Tracked(int v) : value(v) { ++alive; }
    Tracked(const Tracked& o) : value(o.value) {
        ++alive;
        ++copies;
    }
    Tracked(Tracked&& o) noexcept : value(o.value) {
        ++alive;
        ++moves;
    }
    Tracked& operator=(const Tracked& o) {
        value = o.value;
        ++copies;
        return *this;
    }
    Tracked& operator=(Tracked&& o) noexcept {
        value = o.value;
        ++moves;
        return *this;
    }
    ~Tracked() { --alive; }
    static void reset() { alive = copies = moves = default_ctors = 0; }
};

// Move constructor may throw, so a vector must copy on reallocation to keep
// the strong exception guarantee.
struct ThrowingMove {
    static inline int copies = 0;
    static inline int moves = 0;
    int value;
    explicit ThrowingMove(int v) : value(v) {}
    ThrowingMove(const ThrowingMove& o) : value(o.value) { ++copies; }
    ThrowingMove(ThrowingMove&& o) : value(o.value) { ++moves; }  // NOT noexcept
};

// Copy throws on the Nth copy.
struct Bomb {
    static inline int countdown = -1;
    static inline int alive = 0;
    int value;
    explicit Bomb(int v) : value(v) { ++alive; }
    Bomb(const Bomb& o) : value(o.value) {
        if (countdown >= 0 && countdown-- == 0) throw std::runtime_error("boom");
        ++alive;
    }
    ~Bomb() { --alive; }
};

struct NoDefault {
    int v;
    explicit NoDefault(int x) : v(x) {}
    NoDefault() = delete;
};

struct alignas(64) CacheLine {
    std::uint64_t words[8];
};

}  // namespace

TEST(default_is_empty) {
    Vector<int> v;
    CHECK_EQ(v.size(), 0u);
    CHECK_EQ(v.capacity(), 0u);
    CHECK(v.empty());
    CHECK(v.begin() == v.end());
}

TEST(push_back_and_index) {
    Vector<int> v;
    for (int i = 0; i < 100; ++i) v.push_back(i);
    CHECK_EQ(v.size(), 100u);
    CHECK(v.capacity() >= 100u);
    for (int i = 0; i < 100; ++i) CHECK_EQ(v[i], i);
    CHECK_EQ(v.front(), 0);
    CHECK_EQ(v.back(), 99);
}

TEST(growth_is_geometric) {
    Vector<int> v;
    int reallocations = 0;
    const int* last = nullptr;
    for (int i = 0; i < 100000; ++i) {
        v.push_back(i);
        if (v.data() != last) {
            ++reallocations;
            last = v.data();
        }
    }
    // Growing by a constant factor gives O(log n) reallocations and amortised
    // O(1) push_back. Growing by a constant amount would be O(n).
    CHECK(reallocations <= 40);
}

TEST(sized_constructor_value_initializes) {
    Vector<int> v(50);
    CHECK_EQ(v.size(), 50u);
    for (int x : v) CHECK_EQ(x, 0);
    Vector<std::string> s(3, "abc");
    CHECK_EQ(s.size(), 3u);
    CHECK_EQ(s[2], std::string("abc"));
}

TEST(initializer_list) {
    Vector<int> v{1, 2, 3};
    CHECK_EQ(v.size(), 3u);
    CHECK_EQ(v[0], 1);
    CHECK_EQ(v[2], 3);
}

TEST(reserve_does_not_construct) {
    Tracked::reset();
    {
        Vector<Tracked> v;
        v.reserve(1000);
        CHECK(v.capacity() >= 1000u);
        CHECK_EQ(v.size(), 0u);
        CHECK_EQ(Tracked::alive, 0);
        CHECK_EQ(Tracked::default_ctors, 0);
    }
    Vector<NoDefault> nd;
    nd.reserve(10);
    nd.push_back(NoDefault(4));
    CHECK_EQ(nd[0].v, 4);
}

TEST(at_checks_bounds) {
    Vector<int> v{1, 2, 3};
    CHECK_EQ(v.at(2), 3);
    CHECK_THROWS_AS(v.at(3), std::out_of_range);
    const Vector<int>& cv = v;
    CHECK_THROWS_AS(cv.at(100), std::out_of_range);
}

TEST(destroys_everything) {
    Tracked::reset();
    {
        Vector<Tracked> v;
        for (int i = 0; i < 37; ++i) v.emplace_back(i);
        v.pop_back();
        CHECK_EQ(Tracked::alive, 36);
    }
    CHECK_EQ(Tracked::alive, 0);
}

TEST(copy_is_deep) {
    Vector<std::string> a{"x", "y"};
    Vector<std::string> b(a);
    b[0] = "changed";
    CHECK_EQ(a[0], std::string("x"));
    Vector<std::string> c;
    c = a;
    CHECK(c == a);
    Vector<std::string>& alias = c;
    c = alias;  // self-assignment
    CHECK(c == a);
}

TEST(move_steals_buffer) {
    Tracked::reset();
    Vector<Tracked> a;
    for (int i = 0; i < 10; ++i) a.emplace_back(i);
    const Tracked* buf = a.data();
    int moves_before = Tracked::moves;
    Vector<Tracked> b(std::move(a));
    CHECK(b.data() == buf);
    CHECK_EQ(Tracked::moves, moves_before);  // no element-wise moves
    CHECK_EQ(b.size(), 10u);
    CHECK_EQ(a.size(), 0u);
    Vector<Tracked> c;
    c = std::move(b);
    CHECK(c.data() == buf);
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<Vector<Tracked>>);
    CHECK_TRAIT(std::is_nothrow_move_assignable_v<Vector<Tracked>>);
}

TEST(reallocation_moves_noexcept_types) {
    Tracked::reset();
    Vector<Tracked> v;
    for (int i = 0; i < 1000; ++i) v.push_back(Tracked(i));
    CHECK_EQ(Tracked::copies, 0);
    for (int i = 0; i < 1000; ++i) CHECK_EQ(v[i].value, i);
}

TEST(reallocation_copies_when_move_may_throw) {
    ThrowingMove::copies = ThrowingMove::moves = 0;
    Vector<ThrowingMove> v;
    v.reserve(1);
    v.emplace_back(1);
    v.emplace_back(2);  // forces reallocation of 1 existing element
    CHECK_EQ(ThrowingMove::copies, 1);
    CHECK_EQ(ThrowingMove::moves, 0);
}

TEST(move_only_types) {
    Vector<std::unique_ptr<int>> v;
    for (int i = 0; i < 20; ++i) v.push_back(std::make_unique<int>(i));
    CHECK_EQ(*v[19], 19);
    Vector<std::unique_ptr<int>> w(std::move(v));
    CHECK_EQ(*w[0], 0);
}

TEST(strong_guarantee_on_push_back) {
    Bomb::alive = 0;
    {
        Vector<Bomb> v;
        v.reserve(4);
        for (int i = 0; i < 4; ++i) v.emplace_back(i);
        const Bomb* buf = v.data();
        Bomb extra(99);
        Bomb::countdown = 2;  // third copy during reallocation throws
        CHECK_THROWS_AS(v.push_back(extra), std::runtime_error);
        Bomb::countdown = -1;
        // Nothing changed.
        CHECK_EQ(v.size(), 4u);
        CHECK_EQ(v.capacity(), 4u);
        CHECK(v.data() == buf);
        for (int i = 0; i < 4; ++i) CHECK_EQ(v[i].value, i);
        CHECK_EQ(Bomb::alive, 5);  // 4 in the vector + extra
    }
    CHECK_EQ(Bomb::alive, 0);
}

TEST(strong_guarantee_on_reserve) {
    Bomb::alive = 0;
    {
        Vector<Bomb> v;
        for (int i = 0; i < 5; ++i) v.emplace_back(i);
        auto cap = v.capacity();
        Bomb::countdown = 3;
        CHECK_THROWS_AS(v.reserve(cap * 4), std::runtime_error);
        Bomb::countdown = -1;
        CHECK_EQ(v.capacity(), cap);
        CHECK_EQ(v.size(), 5u);
        CHECK_EQ(Bomb::alive, 5);
    }
    CHECK_EQ(Bomb::alive, 0);
}

TEST(push_back_of_own_element_while_growing) {
    // Classic bug: the argument refers into the buffer being reallocated.
    // ASan reports heap-use-after-free if you free before constructing.
    Vector<std::string> v;
    v.push_back(std::string(100, 'a'));
    for (int i = 0; i < 64; ++i) {
        v.push_back(v[0]);
        v.emplace_back(v.back());
    }
    for (const auto& s : v) CHECK_EQ(s, std::string(100, 'a'));
}

TEST(resize_up_and_down) {
    Tracked::reset();
    {
        Vector<Tracked> v;
        v.resize(10);
        CHECK_EQ(v.size(), 10u);
        CHECK_EQ(Tracked::alive, 10);
        v.resize(3);
        CHECK_EQ(Tracked::alive, 3);
        v.resize(6, Tracked(7));
        CHECK_EQ(v.size(), 6u);
        CHECK_EQ(v[5].value, 7);
        CHECK_EQ(v[0].value, -1);
    }
    CHECK_EQ(Tracked::alive, 0);
}

TEST(clear_keeps_capacity_shrink_releases) {
    Tracked::reset();
    Vector<Tracked> v;
    for (int i = 0; i < 100; ++i) v.emplace_back(i);
    auto cap = v.capacity();
    v.clear();
    CHECK_EQ(Tracked::alive, 0);
    CHECK_EQ(v.capacity(), cap);
    v.emplace_back(1);
    v.shrink_to_fit();
    CHECK_EQ(v.capacity(), 1u);
    CHECK_EQ(v[0].value, 1);
}

TEST(erase_shifts_down) {
    Vector<int> v{0, 1, 2, 3, 4};
    auto it = v.erase(v.begin() + 1);
    CHECK_EQ(*it, 2);
    CHECK_EQ(v.size(), 4u);
    CHECK(v == Vector<int>({0, 2, 3, 4}));
    v.erase(v.end() - 1);
    CHECK(v == Vector<int>({0, 2, 3}));
}

TEST(respects_over_alignment) {
    Vector<CacheLine> v;
    for (int i = 0; i < 17; ++i) {
        v.push_back(CacheLine{});
        CHECK_EQ(reinterpret_cast<std::uintptr_t>(v.data()) % 64, 0u);
    }
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
