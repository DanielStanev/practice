#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace hft {

class Arena {
public:
    explicit Arena(std::size_t capacity_bytes)
        : base_(static_cast<std::byte*>(
              ::operator new(capacity_bytes, std::align_val_t{alignof(std::max_align_t)}))),
          capacity_(capacity_bytes) {}

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    ~Arena() { ::operator delete(base_, std::align_val_t{alignof(std::max_align_t)}); }

    void* allocate(std::size_t bytes, std::size_t align = alignof(std::max_align_t)) noexcept {
        auto base = reinterpret_cast<std::uintptr_t>(base_);
        std::uintptr_t cur = base + offset_;
        std::uintptr_t aligned = (cur + align - 1) & ~(static_cast<std::uintptr_t>(align) - 1);
        std::size_t new_offset = (aligned - base) + bytes;
        if (new_offset > capacity_ || aligned < cur) return nullptr;
        offset_ = new_offset;
        return reinterpret_cast<void*>(aligned);
    }

    template <typename T, typename... Args>
        requires std::is_trivially_destructible_v<T>
    T* create(Args&&... args) {
        void* p = allocate(sizeof(T), alignof(T));
        if (!p) return nullptr;
        return ::new (p) T(std::forward<Args>(args)...);
    }

    void reset() noexcept { offset_ = 0; }
    std::size_t used() const noexcept { return offset_; }
    std::size_t capacity() const noexcept { return capacity_; }
    bool owns(const void* p) const noexcept {
        auto* b = static_cast<const std::byte*>(p);
        return b >= base_ && b < base_ + capacity_;
    }

private:
    std::byte* base_;
    std::size_t capacity_;
    std::size_t offset_ = 0;
};

template <typename T>
class ArenaAllocator {
public:
    using value_type = T;

    explicit ArenaAllocator(Arena& a) noexcept : arena_(&a) {}
    template <typename U>
    ArenaAllocator(const ArenaAllocator<U>& o) noexcept : arena_(o.arena()) {}

    T* allocate(std::size_t n) {
        void* p = arena_->allocate(n * sizeof(T), alignof(T));
        if (!p) throw std::bad_alloc();
        return static_cast<T*>(p);
    }
    void deallocate(T*, std::size_t) noexcept {}

    Arena* arena() const noexcept { return arena_; }

    template <typename U>
    bool operator==(const ArenaAllocator<U>& o) const noexcept {
        return arena_ == o.arena();
    }

private:
    Arena* arena_;
};

template <typename T>
class ObjectPool {
    union Slot {
        Slot* next;
        alignas(T) std::byte storage[sizeof(T)];
    };

public:
    explicit ObjectPool(std::size_t capacity)
        : slots_(static_cast<Slot*>(
              ::operator new(capacity * sizeof(Slot), std::align_val_t{alignof(Slot)}))),
          capacity_(capacity),
          available_(capacity) {
        for (std::size_t i = 0; i < capacity; ++i)
            slots_[i].next = i + 1 < capacity ? &slots_[i + 1] : nullptr;
        free_ = capacity ? &slots_[0] : nullptr;
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    ~ObjectPool() { ::operator delete(slots_, std::align_val_t{alignof(Slot)}); }

    template <typename... Args>
    T* acquire(Args&&... args) {
        Slot* s = free_;
        if (!s) return nullptr;
        free_ = s->next;
        try {
            T* obj = ::new (static_cast<void*>(s->storage)) T(std::forward<Args>(args)...);
            --available_;
            return obj;
        } catch (...) {
            s->next = free_;
            free_ = s;
            throw;
        }
    }

    void release(T* p) noexcept {
        std::destroy_at(p);
        Slot* s = reinterpret_cast<Slot*>(p);
        s->next = free_;
        free_ = s;
        ++available_;
    }

    std::size_t capacity() const noexcept { return capacity_; }
    std::size_t available() const noexcept { return available_; }
    bool owns(const T* p) const noexcept {
        auto* b = reinterpret_cast<const std::byte*>(p);
        auto* lo = reinterpret_cast<const std::byte*>(slots_);
        auto* hi = reinterpret_cast<const std::byte*>(slots_ + capacity_);
        return b >= lo && b < hi && (b - lo) % sizeof(Slot) == 0;
    }

private:
    Slot* slots_;
    Slot* free_ = nullptr;
    std::size_t capacity_;
    std::size_t available_;
};

}  // namespace hft
