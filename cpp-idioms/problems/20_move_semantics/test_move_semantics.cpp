#include "move_semantics.hpp"

#include <cstdlib>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

// Count array allocations (Buffer uses new char[]).
namespace {
long g_array_news = 0;
}
void* operator new[](std::size_t n) {
    ++g_array_news;
    if (void* p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"  // false positive: these pair with the operator new[] above
#endif
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

namespace {
struct Tracked {
    static inline int copies = 0, moves = 0;
    std::string v;
    Tracked() = default;
    Tracked(const char* s) : v(s) {}
    Tracked(const Tracked& o) : v(o.v) { ++copies; }
    Tracked(Tracked&& o) noexcept : v(std::move(o.v)) { ++moves; }
    Tracked& operator=(const Tracked& o) { v = o.v; ++copies; return *this; }
    Tracked& operator=(Tracked&& o) noexcept { v = std::move(o.v); ++moves; return *this; }
    static void reset() { copies = moves = 0; }
};
}  // namespace

TEST(buffer_traits) {
    CHECK_TRAIT(std::is_copy_constructible_v<Buffer>);
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<Buffer>);
    CHECK_TRAIT(std::is_nothrow_move_assignable_v<Buffer>);
    CHECK_TRAIT(std::is_nothrow_swappable_v<Buffer>);
    Buffer a(4);  // the implicit members also have these traits, so check behaviour too
    Buffer b(std::move(a));
    CHECK(a.data() == nullptr);
    CHECK_EQ(b.size(), std::size_t{4});
}

TEST(buffer_copy_is_deep) {
    Buffer a(16);
    CHECK_EQ(a.data()[15], char{0});
    a.data()[0] = 'x';
    long before = g_array_news;
    Buffer b(a);
    CHECK_EQ(g_array_news, before + 1);
    CHECK(b.data() != a.data());
    CHECK_EQ(b.data()[0], 'x');
    b.data()[0] = 'y';
    CHECK_EQ(a.data()[0], 'x');
    Buffer c(4);
    c = a;
    CHECK_EQ(c.size(), std::size_t{16});
    CHECK_EQ(c.data()[0], 'x');
}

TEST(buffer_move_steals) {
    Buffer a(32);
    char* p = a.data();
    long before = g_array_news;
    Buffer b(std::move(a));
    CHECK_EQ(g_array_news, before);
    CHECK(b.data() == p);
    CHECK_EQ(b.size(), std::size_t{32});
    CHECK(a.data() == nullptr);
    CHECK_EQ(a.size(), std::size_t{0});
    Buffer c(8);
    c = std::move(b);
    CHECK(c.data() == p);
    CHECK(b.data() == nullptr);
    a = std::move(c);  // moved-from objects can be assigned to
    CHECK(a.data() == p);
}

TEST(buffer_self_assignment) {
    Buffer a(8);
    a.data()[3] = 'z';
    Buffer& alias = a;
    a = alias;
    CHECK_EQ(a.size(), std::size_t{8});
    CHECK_EQ(a.data()[3], 'z');
    a = std::move(alias);
    CHECK_EQ(a.size(), std::size_t{8});
    CHECK_EQ(a.data()[3], 'z');
}

TEST(buffer_swap_and_vector_growth) {
    Buffer a(1), b(2);
    using std::swap;
    swap(a, b);
    CHECK_EQ(a.size(), std::size_t{2});
    CHECK_EQ(b.size(), std::size_t{1});
    std::vector<Buffer> v;
    for (int i = 0; i < 100; ++i) v.emplace_back(10);
    long before = g_array_news;
    v.shrink_to_fit();
    v.emplace_back(10);  // reallocation must move, not copy, the 100 buffers
    CHECK_EQ(g_array_news, before + 1);
}

TEST(holder_sink_parameters) {
    Tracked t("abc");
    Tracked::reset();
    Holder<Tracked> h(t);  // one copy into the parameter, then a move
    CHECK_EQ(Tracked::copies, 1);
    Tracked::reset();
    h.set(std::move(t));
    CHECK_EQ(Tracked::copies, 0);
    Tracked::reset();
    h.set("xyz");
    CHECK_EQ(Tracked::copies, 0);
    CHECK_EQ(h.get().v, std::string("xyz"));
}

TEST(holder_ref_qualified_get) {
    Holder<Tracked> h(Tracked("payload"));
    Tracked::reset();
    const Tracked& ref = h.get();
    CHECK_EQ(Tracked::copies + Tracked::moves, 0);
    CHECK_EQ(ref.v, std::string("payload"));
    Tracked stolen = std::move(h).get();
    CHECK_EQ(Tracked::copies, 0);
    CHECK_EQ(stolen.v, std::string("payload"));
    Tracked::reset();
    Tracked from_temp = Holder<Tracked>(Tracked("tmp")).get();
    CHECK_EQ(Tracked::copies, 0);
    CHECK_EQ(from_temp.v, std::string("tmp"));
}

TEST(append_reuses_buffer) {
    std::vector<Tracked> base{"a", "b", "c"};
    base.reserve(10);
    const Tracked* old = base.data();
    std::vector<Tracked> extra{"d", "e"};
    Tracked::reset();
    auto r = append(std::move(base), extra);
    CHECK_EQ(Tracked::copies, 2);  // only the extras
    CHECK(r.data() == old);        // same buffer, no reallocation
    CHECK_EQ(r.size(), std::size_t{5});
    CHECK_EQ(r[4].v, std::string("e"));
    std::vector<Tracked> keep{"x"};
    auto r2 = append(keep, extra);  // lvalue: caller keeps theirs
    CHECK_EQ(keep.size(), std::size_t{1});
    CHECK_EQ(r2.size(), std::size_t{3});
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
