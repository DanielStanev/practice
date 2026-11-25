#include "noexcept_moves.hpp"

#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {

struct Counted {
    static inline int copies = 0, moves = 0, alive = 0;
    int v;
    explicit Counted(int x) : v(x) { ++alive; }
    Counted(const Counted& o) : v(o.v) { ++copies; ++alive; }
    Counted(Counted&& o) noexcept : v(o.v) { ++moves; ++alive; }
    ~Counted() { --alive; }
    static void reset() { copies = moves = 0; }
};

// Copyable, but its move may throw: relocation must copy.
struct RiskyMove {
    static inline int copies = 0, moves = 0, alive = 0;
    static inline int copies_until_throw = -1;
    int v;
    explicit RiskyMove(int x) : v(x) { ++alive; }
    RiskyMove(const RiskyMove& o) : v(o.v) {
        if (copies_until_throw == 0) throw std::runtime_error("copy failed");
        if (copies_until_throw > 0) --copies_until_throw;
        ++copies;
        ++alive;
    }
    RiskyMove(RiskyMove&& o) : v(o.v) { ++moves; ++alive; }
    ~RiskyMove() { --alive; }
};

// Move-only with a throwing move: there is no choice but to move.
struct MoveOnlyRisky {
    static inline int moves = 0;
    std::unique_ptr<int> p;
    explicit MoveOnlyRisky(int x) : p(std::make_unique<int>(x)) {}
    MoveOnlyRisky(MoveOnlyRisky&& o) : p(std::move(o.p)) { ++moves; }
};

template <class T>
struct Raw {
    alignas(T) unsigned char bytes[sizeof(T) * 8];
    T* get() { return reinterpret_cast<T*>(bytes); }
};

}  // namespace

TEST(message_moves_on_growth) {
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<Message>);
    CHECK_TRAIT(std::is_nothrow_move_assignable_v<Message>);
    std::vector<Message> v;
    v.emplace_back("t", std::vector<int>{1, 2, 3});
    v.shrink_to_fit();
    Message::moves = 0;
    v.emplace_back("u", std::vector<int>{4});  // grows: the existing element must be moved
    CHECK_EQ(Message::moves, 1);
}

TEST(box_conditional_noexcept) {
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<Box<std::string>>);
    CHECK_TRAIT(std::is_nothrow_move_assignable_v<Box<std::string>>);
    CHECK_TRAIT(!std::is_nothrow_move_constructible_v<Box<RiskyMove>>);
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<Box<int>>);
}

TEST(relocate_moves_when_noexcept) {
    Raw<Counted> a, b;
    for (int i = 0; i < 5; ++i) std::construct_at(a.get() + i, i);
    Counted::reset();
    relocate(a.get(), b.get(), 5);
    CHECK_EQ(Counted::moves, 5);
    CHECK_EQ(Counted::copies, 0);
    CHECK_EQ(Counted::alive, 5);  // sources destroyed
    CHECK_EQ(b.get()[4].v, 4);
    std::destroy_n(b.get(), 5);
}

TEST(relocate_copies_when_move_may_throw) {
    Raw<RiskyMove> a, b;
    for (int i = 0; i < 4; ++i) std::construct_at(a.get() + i, i);
    RiskyMove::copies = RiskyMove::moves = 0;
    relocate(a.get(), b.get(), 4);
    CHECK_EQ(RiskyMove::copies, 4);
    CHECK_EQ(RiskyMove::moves, 0);
    CHECK_EQ(RiskyMove::alive, 4);
    std::destroy_n(b.get(), 4);
}

TEST(relocate_rolls_back_on_throw) {
    Raw<RiskyMove> a, b;
    for (int i = 0; i < 6; ++i) std::construct_at(a.get() + i, i * 10);
    RiskyMove::copies_until_throw = 3;
    bool threw = false;
    try {
        relocate(a.get(), b.get(), 6);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    RiskyMove::copies_until_throw = -1;
    CHECK(threw);
    CHECK_EQ(RiskyMove::alive, 6);  // the 3 copies were destroyed, the 6 sources live on
    CHECK_EQ(a.get()[5].v, 50);
    std::destroy_n(a.get(), 6);
}

TEST(relocate_move_only_must_move) {
    Raw<MoveOnlyRisky> a, b;
    for (int i = 0; i < 3; ++i) std::construct_at(a.get() + i, i);
    MoveOnlyRisky::moves = 0;
    relocate(a.get(), b.get(), 3);
    CHECK_EQ(MoveOnlyRisky::moves, 3);
    CHECK_EQ(*b.get()[2].p, 2);
    std::destroy_n(b.get(), 3);
}

TEST(take_last_no_copy) {
    std::vector<Counted> v;
    v.reserve(3);
    v.emplace_back(1);
    v.emplace_back(2);
    Counted::reset();
    Counted c = take_last(std::move(v));
    CHECK_EQ(Counted::copies, 0);
    CHECK_EQ(c.v, 2);
}

TEST(name_of_no_copy) {
    std::pair<Counted, int> p{Counted(7), 1};
    Counted::reset();
    Counted c = name_of(std::move(p));
    CHECK_EQ(Counted::copies, 0);
    CHECK_EQ(c.v, 7);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
