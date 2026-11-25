// Reference solution for problem 30.
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

struct AllDefaults {
    Tracker t;
};

struct CopiesWhenMoved {
    Tracker t;
    ~CopiesWhenMoved() {}  // suppresses the implicit move operations
};

struct DestructorButMoves {
    Tracker t;
    DestructorButMoves() = default;
    DestructorButMoves(const DestructorButMoves&) = default;
    DestructorButMoves(DestructorButMoves&&) = default;
    DestructorButMoves& operator=(const DestructorButMoves&) = default;
    DestructorButMoves& operator=(DestructorButMoves&&) = default;
    ~DestructorButMoves() {}
};

struct MoveOnlyByMember {
    Tracker t;
    std::unique_ptr<int> p;  // deletes the implicit copies
};

struct AssignDeleted {
    Tracker t;
    const int id = 0;  // deletes the implicit assignments
};

struct NoDefaultCtor {
    Tracker t;
    explicit NoDefaultCtor(int) {}  // any constructor suppresses the default one
};

struct Aggregate {
    int id;
    double px;
    Tracker t;
};

struct Trivial {
    int a;
    int b;
};

struct NotMovable {
    Tracker t;
    NotMovable() = default;
    NotMovable(const NotMovable&) = default;
    NotMovable(NotMovable&&) = delete;  // declared and deleted: wins overload resolution for rvalues, then fails
};

struct MoveCtorOnly {
    Tracker t;
    MoveCtorOnly() = default;
    MoveCtorOnly(MoveCtorOnly&&) = default;  // copies become deleted
};

struct RefMember {
    Tracker t;
    int& r;
    explicit RefMember(int& x) : r(x) {}
};

}  // namespace idioms
