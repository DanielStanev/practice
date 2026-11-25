// Problem 07: Arena (bump) allocator and fixed-size object pool
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hft {

// Hands out memory from one up-front buffer by bumping an offset.
// Individual allocations are never freed; reset() frees everything at once.
class Arena {
public:
    // Allocates the whole buffer once, here.
    explicit Arena(std::size_t capacity_bytes) : capacity_(capacity_bytes) {
        // TODO
    }

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    ~Arena() {
        // TODO
    }

    // Returns `bytes` bytes aligned to `align` (a power of two), or nullptr if
    // there is not enough room left. A failed call changes nothing.
    void* allocate(std::size_t bytes, std::size_t align = alignof(std::max_align_t)) noexcept {
        (void)bytes;
        (void)align;
        return nullptr;  // TODO
    }

    // Allocates and constructs a T, or returns nullptr if full.
    // The arena never runs destructors, so only trivially destructible T are
    // allowed. TODO: enforce that with a constraint.
    template <typename T, typename... Args>
    T* create(Args&&... args) {
        ((void)args, ...);
        throw std::logic_error("TODO: Arena::create");
    }

    void reset() noexcept {
        // TODO
    }

    std::size_t used() const noexcept { return 0; /* TODO */ }
    std::size_t capacity() const noexcept { return capacity_; }

    // True if p points into this arena's buffer.
    bool owns(const void* p) const noexcept {
        (void)p;
        return false;  // TODO
    }

private:
    std::size_t capacity_;
};

// Standard-library compatible allocator that draws from an Arena.
// deallocate() is a no-op. Allocation failure throws std::bad_alloc.
template <typename T>
class ArenaAllocator {
public:
    using value_type = T;

    explicit ArenaAllocator(Arena& a) noexcept : arena_(&a) {}

    // Rebinding constructor: containers convert ArenaAllocator<T> to
    // ArenaAllocator<Node> internally.
    template <typename U>
    ArenaAllocator(const ArenaAllocator<U>& o) noexcept : arena_(o.arena()) {}

    T* allocate(std::size_t n) {
        (void)n;
        throw std::logic_error("TODO: ArenaAllocator::allocate");
    }
    void deallocate(T*, std::size_t) noexcept {}

    Arena* arena() const noexcept { return arena_; }

    // Two allocators are equal if memory from one can be freed by the other.
    template <typename U>
    bool operator==(const ArenaAllocator<U>& o) const noexcept {
        (void)o;
        return false;  // TODO
    }

private:
    Arena* arena_;
};

// Fixed number of T-sized slots allocated up front. acquire()/release() are
// O(1) and never touch the heap.
template <typename T>
class ObjectPool {
public:
    explicit ObjectPool(std::size_t capacity) : capacity_(capacity) {
        // TODO: allocate all slots and build the free list.
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // Frees the slots. Objects still acquired are not destroyed (caller's bug).
    ~ObjectPool() {
        // TODO
    }

    // Constructs a T in a free slot, or returns nullptr if none is free.
    // If T's constructor throws, the slot goes back to the pool.
    template <typename... Args>
    T* acquire(Args&&... args) {
        ((void)args, ...);
        throw std::logic_error("TODO: ObjectPool::acquire");
    }

    // Destroys *p and returns its slot. The next acquire() reuses it (LIFO).
    void release(T* p) noexcept {
        (void)p;
        // TODO
    }

    std::size_t capacity() const noexcept { return capacity_; }
    std::size_t available() const noexcept { return 0; /* TODO */ }

    // True if p is the address of one of this pool's slots.
    bool owns(const T* p) const noexcept {
        (void)p;
        return false;  // TODO
    }

private:
    std::size_t capacity_;
};

}  // namespace hft
