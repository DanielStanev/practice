// Problem 30: which special members does the compiler give you?
// Read README.md first. Edit each struct to match its description.
// Do not use `= delete` unless the description says so.
#pragma once

#include <memory>
#include <string>

namespace idioms {

struct Tracker {
    static inline int copies = 0;
    static inline int moves = 0;
    Tracker() = default;
    Tracker(const Tracker&) { ++copies; }
    Tracker(Tracker&&) noexcept { ++moves; }
    Tracker& operator=(const Tracker&) { ++copies; return *this; }
    Tracker& operator=(Tracker&&) noexcept { ++moves; return *this; }
    static void reset() { copies = moves = 0; }
};

// 1. Default constructible, copyable, movable; moving really moves.
//    (Already correct as given. Make sure you can say why.)
struct AllDefaults {
    Tracker t;
};

// 2. Has a user-declared destructor (`~CopiesWhenMoved() {}`), and as a
//    consequence std::move(x) COPIES. Keep both properties: this one is a
//    demonstration of the trap.
struct CopiesWhenMoved {
    Tracker t;
};

// 3. Same destructor as (2), but moving moves. (Fix the trap.)
struct DestructorButMoves {
    Tracker t;
};

// 4. Move-only: movable, not copyable. No `= delete`, no special members:
//    choose a member type that makes it so.
struct MoveOnlyByMember {
    Tracker t;
};

// 5. Copy-constructible but NOT copy- or move-assignable. No special members.
struct AssignDeleted {
    Tracker t;
};

// 6. Not default constructible, but copyable and movable. No `= delete`.
struct NoDefaultCtor {
    Tracker t;
};

// 7. An aggregate: it can be initialised as `Aggregate{.id = 1, .px = 2.0}`.
//    (It may keep Tracker t as its last member.)
struct Aggregate {
    Tracker t;
};

// 8. Trivially copyable and trivially default constructible, standard layout.
//    This one must NOT contain a Tracker (Tracker is not trivial). Two ints.
struct Trivial {};

// 9. Copyable, and NOT move-constructible at all: constructing from an
//    rvalue does not compile. You may use `= delete` here.
struct NotMovable {
    Tracker t;
};

// 10. Declares a move constructor (moving works and moves), and as a
//     consequence is not copyable. No `= delete`.
struct MoveCtorOnly {
    Tracker t;
};

// 11. Has a reference member `int& r`; copy-constructible, not assignable,
//     not default constructible. Provide a constructor from int&.
struct RefMember {
    Tracker t;
};

}  // namespace idioms
