#include "unique_ptr.hpp"

#include <string>
#include <type_traits>
#include <utility>

#include "testing.hpp"

using hft::UniquePtr;

namespace {

struct Tracked {
    static inline int alive = 0;
    static inline int destroyed = 0;
    int value;
    explicit Tracked(int v = 0) : value(v) { ++alive; }
    Tracked(const Tracked&) = delete;
    ~Tracked() {
        --alive;
        ++destroyed;
    }
    static void reset_counters() {
        alive = 0;
        destroyed = 0;
    }
};

struct Base {
    virtual ~Base() = default;
    virtual int id() const { return 1; }
};
struct Derived : Base {
    static inline int destroyed = 0;
    ~Derived() override { ++destroyed; }
    int id() const override { return 2; }
};

// Stateful deleter: counts how many times it ran, through a pointer so the
// count survives moves of the deleter.
struct CountingDeleter {
    int* calls = nullptr;
    void operator()(Tracked* p) const noexcept {
        ++*calls;
        delete p;
    }
};

int g_fn_deleter_calls = 0;
void fn_deleter(Tracked* p) {
    ++g_fn_deleter_calls;
    delete p;
}

}  // namespace

TEST(default_constructed_is_empty) {
    UniquePtr<int> p;
    CHECK(p.get() == nullptr);
    CHECK_FALSE(static_cast<bool>(p));
    CHECK(p == nullptr);
}

TEST(owns_and_destroys) {
    Tracked::reset_counters();
    {
        UniquePtr<Tracked> p(new Tracked(42));
        CHECK(static_cast<bool>(p));
        CHECK_EQ(p->value, 42);
        CHECK_EQ((*p).value, 42);
        CHECK_EQ(Tracked::alive, 1);
    }
    CHECK_EQ(Tracked::alive, 0);
    CHECK_EQ(Tracked::destroyed, 1);
}

TEST(make_unique_forwards_arguments) {
    auto s = hft::make_unique<std::string>(5, 'x');
    CHECK_EQ(*s, std::string("xxxxx"));
}

TEST(is_move_only) {
    CHECK_TRAIT(!std::is_copy_constructible_v<UniquePtr<int>>);
    CHECK_TRAIT(!std::is_copy_assignable_v<UniquePtr<int>>);
    CHECK_TRAIT(std::is_move_constructible_v<UniquePtr<int>>);
    CHECK_TRAIT(std::is_move_assignable_v<UniquePtr<int>>);
}

TEST(moves_are_noexcept) {
    // std::vector<UniquePtr<T>> only moves (instead of failing to copy) on
    // reallocation if the move constructor is noexcept.
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<UniquePtr<int>>);
    CHECK_TRAIT(std::is_nothrow_move_assignable_v<UniquePtr<int>>);
}

TEST(zero_overhead_with_empty_deleter) {
    CHECK_EQ(sizeof(UniquePtr<int>), sizeof(int*));
    CHECK_EQ(sizeof(UniquePtr<Tracked>), sizeof(Tracked*));
}

TEST(move_construct_transfers_ownership) {
    Tracked::reset_counters();
    UniquePtr<Tracked> a(new Tracked(7));
    Tracked* raw = a.get();
    UniquePtr<Tracked> b(std::move(a));
    CHECK(a.get() == nullptr);
    CHECK(b.get() == raw);
    CHECK_EQ(Tracked::alive, 1);
}

TEST(move_assign_destroys_previous) {
    Tracked::reset_counters();
    UniquePtr<Tracked> a(new Tracked(1));
    UniquePtr<Tracked> b(new Tracked(2));
    b = std::move(a);
    CHECK_EQ(Tracked::destroyed, 1);
    CHECK_EQ(Tracked::alive, 1);
    CHECK_EQ(b->value, 1);
    CHECK(a.get() == nullptr);
}

TEST(self_move_assign_is_safe) {
    Tracked::reset_counters();
    UniquePtr<Tracked> a(new Tracked(3));
    UniquePtr<Tracked>& alias = a;
    a = std::move(alias);
    CHECK(a.get() != nullptr);
    CHECK_EQ(a->value, 3);
    CHECK_EQ(Tracked::alive, 1);
}

TEST(release_gives_up_ownership) {
    Tracked::reset_counters();
    UniquePtr<Tracked> p(new Tracked(5));
    Tracked* raw = p.release();
    CHECK(p.get() == nullptr);
    CHECK_EQ(Tracked::alive, 1);
    delete raw;
    CHECK_EQ(Tracked::alive, 0);
}

TEST(reset_replaces_and_destroys_old) {
    Tracked::reset_counters();
    UniquePtr<Tracked> p(new Tracked(1));
    p.reset(new Tracked(2));
    CHECK_EQ(p->value, 2);
    CHECK_EQ(Tracked::destroyed, 1);
    p.reset();
    CHECK(p.get() == nullptr);
    CHECK_EQ(Tracked::alive, 0);
}

namespace {
// The standard requires reset() to store the new pointer *before* deleting
// the old one. An object that observes its owner during destruction can see
// the difference.
struct Observer;
UniquePtr<Observer>* g_owner = nullptr;
bool g_saw_new_pointer = false;
struct Observer {
    ~Observer() { g_saw_new_pointer = (g_owner->get() != this); }
};
}  // namespace

TEST(reset_updates_pointer_before_deleting) {
    UniquePtr<Observer> p(new Observer);
    g_owner = &p;
    g_saw_new_pointer = false;
    p.reset();
    CHECK(g_saw_new_pointer);
}

TEST(assign_nullptr_resets) {
    Tracked::reset_counters();
    UniquePtr<Tracked> p(new Tracked(1));
    p = nullptr;
    CHECK(p.get() == nullptr);
    CHECK_EQ(Tracked::alive, 0);
}

TEST(swap_exchanges_pointers) {
    UniquePtr<Tracked> a(new Tracked(1));
    UniquePtr<Tracked> b(new Tracked(2));
    a.swap(b);
    CHECK_EQ(a->value, 2);
    CHECK_EQ(b->value, 1);
}

TEST(converts_derived_to_base) {
    Derived::destroyed = 0;
    {
        UniquePtr<Derived> d(new Derived);
        UniquePtr<Base> b(std::move(d));
        CHECK(d.get() == nullptr);
        CHECK_EQ(b->id(), 2);
        UniquePtr<Base> c;
        c = hft::make_unique<Derived>();
        CHECK_EQ(c->id(), 2);
    }
    CHECK_EQ(Derived::destroyed, 2);
    CHECK_TRAIT(!std::is_constructible_v<UniquePtr<Derived>, UniquePtr<Base>&&>);
}

TEST(stateful_custom_deleter) {
    int calls = 0;
    {
        UniquePtr<Tracked, CountingDeleter> p(new Tracked(1), CountingDeleter{&calls});
        CHECK(p.get_deleter().calls == &calls);
        UniquePtr<Tracked, CountingDeleter> q(std::move(p));
        CHECK_EQ(calls, 0);
    }
    CHECK_EQ(calls, 1);
}

TEST(function_pointer_deleter) {
    g_fn_deleter_calls = 0;
    {
        UniquePtr<Tracked, void (*)(Tracked*)> p(new Tracked(1), &fn_deleter);
        p.reset(new Tracked(2));
        CHECK_EQ(g_fn_deleter_calls, 1);
    }
    CHECK_EQ(g_fn_deleter_calls, 2);
}

TEST(deleter_not_called_on_null) {
    int calls = 0;
    {
        UniquePtr<Tracked, CountingDeleter> p(nullptr, CountingDeleter{&calls});
        p.reset();
    }
    CHECK_EQ(calls, 0);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
