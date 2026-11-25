// Counts heap allocations made through global operator new, so tests can
// assert that a hot path does not allocate. Include from exactly one
// translation unit (the test file).
//
//   long before = alloc_counter::count();
//   ... code under test ...
//   CHECK_EQ(alloc_counter::count(), before);
#pragma once

#include <atomic>
#include <cstdlib>
#include <new>

namespace alloc_counter {
inline std::atomic<long> g_count{0};
inline long count() { return g_count.load(std::memory_order_relaxed); }
inline void* allocate(std::size_t n) {
    g_count.fetch_add(1, std::memory_order_relaxed);
    return std::malloc(n ? n : 1);
}
}  // namespace alloc_counter

// GCC inlines these replacements and then warns that free() is paired with
// operator new. That is exactly the pairing we intend here.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif

// Replace every non-aligned form so new/delete always pair up.
// (Over-aligned forms are left to the runtime and are not counted.)
void* operator new(std::size_t n) {
    if (void* p = alloc_counter::allocate(n)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t n) {
    if (void* p = alloc_counter::allocate(n)) return p;
    throw std::bad_alloc();
}
void* operator new(std::size_t n, const std::nothrow_t&) noexcept { return alloc_counter::allocate(n); }
void* operator new[](std::size_t n, const std::nothrow_t&) noexcept { return alloc_counter::allocate(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
void operator delete(void* p, const std::nothrow_t&) noexcept { std::free(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept { std::free(p); }
