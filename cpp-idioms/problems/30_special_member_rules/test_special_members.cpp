#include "special_members.hpp"

#include <type_traits>
#include <utility>

#include "testing.hpp"

using namespace idioms;
using namespace std;

namespace {
// Builds a T from an rvalue (if possible) and reports what Tracker saw.
template <class T, class... Args>
std::pair<int, int> move_construct_counts(Args&... args) {
    if constexpr (is_constructible_v<T, Args&...> && is_move_constructible_v<T>) {
        T a(args...);
        Tracker::reset();
        T b(std::move(a));
        return {Tracker::copies, Tracker::moves};
    } else {
        return {-1, -1};
    }
}
template <class T>
concept DesignatedInit = requires { T{.id = 1, .px = 2.0}; };
}  // namespace

TEST(t01_all_defaults) {
    CHECK_TRAIT(is_default_constructible_v<AllDefaults> && is_copy_constructible_v<AllDefaults> &&
                is_copy_assignable_v<AllDefaults> && is_nothrow_move_constructible_v<AllDefaults>);
    CHECK(move_construct_counts<AllDefaults>() == std::make_pair(0, 1));
}

TEST(t02_copies_when_moved) {
    CHECK_TRAIT(!is_trivially_destructible_v<CopiesWhenMoved>);
    CHECK_TRAIT(is_move_constructible_v<CopiesWhenMoved>);  // still "movable"...
    CHECK(move_construct_counts<CopiesWhenMoved>() == std::make_pair(1, 0));  // ...but it copies
}

TEST(t03_destructor_but_moves) {
    CHECK_TRAIT(!is_trivially_destructible_v<DestructorButMoves>);
    CHECK_TRAIT(is_copy_constructible_v<DestructorButMoves>);
    CHECK_TRAIT(is_copy_assignable_v<DestructorButMoves>);
    CHECK(move_construct_counts<DestructorButMoves>() == std::make_pair(0, 1));
}

TEST(t04_move_only_by_member) {
    CHECK_TRAIT(!is_copy_constructible_v<MoveOnlyByMember>);
    CHECK_TRAIT(!is_copy_assignable_v<MoveOnlyByMember>);
    CHECK_TRAIT(is_nothrow_move_constructible_v<MoveOnlyByMember>);
    CHECK_TRAIT(is_move_assignable_v<MoveOnlyByMember>);
    CHECK(move_construct_counts<MoveOnlyByMember>() == std::make_pair(0, 1));
}

TEST(t05_assign_deleted) {
    CHECK_TRAIT(is_default_constructible_v<AssignDeleted>);
    CHECK_TRAIT(is_copy_constructible_v<AssignDeleted>);
    CHECK_TRAIT(!is_copy_assignable_v<AssignDeleted>);
    CHECK_TRAIT(!is_move_assignable_v<AssignDeleted>);
}

TEST(t06_no_default_ctor) {
    CHECK_TRAIT(!is_default_constructible_v<NoDefaultCtor>);
    CHECK_TRAIT(is_copy_constructible_v<NoDefaultCtor>);
    CHECK_TRAIT(is_nothrow_move_constructible_v<NoDefaultCtor>);
    CHECK_TRAIT(is_copy_assignable_v<NoDefaultCtor>);
}

TEST(t07_aggregate) {
    CHECK_TRAIT(is_aggregate_v<Aggregate>);
    CHECK_TRAIT(DesignatedInit<Aggregate>);
}

TEST(t08_trivial) {
    CHECK_TRAIT(is_trivially_copyable_v<Trivial>);
    CHECK_TRAIT(is_trivially_default_constructible_v<Trivial>);
    CHECK_TRAIT(is_standard_layout_v<Trivial>);
    CHECK_TRAIT(sizeof(Trivial) == 2 * sizeof(int));
}

TEST(t09_not_movable) {
    CHECK_TRAIT(is_copy_constructible_v<NotMovable>);
    CHECK_TRAIT(!is_move_constructible_v<NotMovable>);
    CHECK_TRAIT(is_default_constructible_v<NotMovable>);
}

TEST(t10_move_ctor_only) {
    CHECK_TRAIT(!is_copy_constructible_v<MoveCtorOnly>);
    CHECK_TRAIT(!is_copy_assignable_v<MoveCtorOnly>);
    CHECK_TRAIT(is_move_constructible_v<MoveCtorOnly>);
    CHECK_TRAIT(is_default_constructible_v<MoveCtorOnly>);
    CHECK(move_construct_counts<MoveCtorOnly>() == std::make_pair(0, 1));
}

TEST(t11_ref_member) {
    CHECK_TRAIT(is_constructible_v<RefMember, int&>);
    CHECK_TRAIT(!is_default_constructible_v<RefMember>);
    CHECK_TRAIT(is_copy_constructible_v<RefMember>);
    CHECK_TRAIT(!is_copy_assignable_v<RefMember>);
    CHECK_TRAIT(!is_constructible_v<RefMember, int>);  // no binding to temporaries
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
