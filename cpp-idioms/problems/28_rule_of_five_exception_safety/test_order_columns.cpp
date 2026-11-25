#include "order_columns.hpp"

#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

#include "testing.hpp"

using idioms::OrderColumns;

// operator new[] that throws on the N-th call when armed.
namespace {
int g_fail_countdown = -1;  // -1: never fail
long g_array_allocs = 0;
long g_array_frees = 0;
}  // namespace
void* operator new[](std::size_t n) {
    if (g_fail_countdown == 0) {
        g_fail_countdown = -1;
        throw std::bad_alloc();
    }
    if (g_fail_countdown > 0) --g_fail_countdown;
    ++g_array_allocs;
    if (void* p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
void operator delete[](void* p) noexcept {
    if (p) ++g_array_frees;
    std::free(p);
}
void operator delete[](void* p, std::size_t) noexcept { ::operator delete[](p); }

namespace {
OrderColumns make(std::size_t n) {
    OrderColumns c(n);
    for (std::size_t i = 0; i < n; ++i) {
        c.price(i) = 100.0 + static_cast<double>(i);
        c.qty(i) = static_cast<long>(i * 10);
    }
    return c;
}
long live_arrays() { return g_array_allocs - g_array_frees; }
}  // namespace

TEST(construct_zeroed) {
    OrderColumns c(4);
    CHECK_EQ(c.size(), std::size_t{4});
    CHECK_EQ(c.price(3), 0.0);
    CHECK_EQ(c.qty(3), 0L);
}

TEST(deep_copy) {
    OrderColumns a = make(5);
    OrderColumns b(a);
    CHECK(b.prices_data() != a.prices_data());
    CHECK_EQ(b.price(4), 104.0);
    b.qty(0) = 999;
    CHECK_EQ(a.qty(0), 0L);
    OrderColumns c(1);
    c = a;
    CHECK_EQ(c.size(), std::size_t{5});
    CHECK_EQ(c.qty(4), 40L);
    OrderColumns& alias = c;
    c = alias;  // self-assignment
    CHECK_EQ(c.qty(4), 40L);
}

TEST(copy_constructor_no_leak_when_second_allocation_throws) {
    OrderColumns a = make(8);
    long before = live_arrays();
    g_fail_countdown = 1;  // first new[] succeeds, second throws
    CHECK_THROWS_AS(OrderColumns(a), std::bad_alloc);
    CHECK_EQ(live_arrays(), before);
}

TEST(constructor_no_leak_when_second_allocation_throws) {
    long before = live_arrays();
    g_fail_countdown = 1;
    CHECK_THROWS_AS(OrderColumns(16), std::bad_alloc);
    CHECK_EQ(live_arrays(), before);
}

TEST(copy_assignment_strong_guarantee) {
    OrderColumns src = make(6);
    OrderColumns dst = make(3);
    const double* old = dst.prices_data();
    for (int fail_at = 0; fail_at < 2; ++fail_at) {
        g_fail_countdown = fail_at;
        CHECK_THROWS_AS(dst = src, std::bad_alloc);
        CHECK_EQ(dst.size(), std::size_t{3});  // unchanged
        CHECK(dst.prices_data() == old);
        CHECK_EQ(dst.price(2), 102.0);
    }
}

TEST(moves_steal) {
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<OrderColumns>);
    CHECK_TRAIT(std::is_nothrow_move_assignable_v<OrderColumns>);
    CHECK_TRAIT(std::is_nothrow_swappable_v<OrderColumns>);
    OrderColumns a = make(3);
    const double* p = a.prices_data();
    long allocs = g_array_allocs;
    OrderColumns b(std::move(a));
    CHECK(b.prices_data() == p);
    CHECK_EQ(a.size(), std::size_t{0});
    CHECK(a.prices_data() == nullptr);
    OrderColumns c = make(2);
    c = std::move(b);
    CHECK(c.prices_data() == p);
    CHECK_EQ(g_array_allocs, allocs + 2);  // only make(2)
}

TEST(vector_of_columns) {
    long before = live_arrays();
    {
        std::vector<OrderColumns> v;
        for (int i = 0; i < 20; ++i) v.push_back(make(4));
        std::vector<OrderColumns> w = v;
        CHECK_EQ(w[19].price(3), 103.0);
    }
    CHECK_EQ(live_arrays(), before);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
