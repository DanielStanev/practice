#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <utility>

namespace hft {

namespace detail {

struct ControlBlockBase {
    std::atomic<long> strong{1};
    // Number of WeakPtrs, plus one collectively held by all SharedPtrs.
    std::atomic<long> weak{1};

    virtual void destroy_object() noexcept = 0;
    virtual void destroy_self() noexcept = 0;

    void add_strong() noexcept { strong.fetch_add(1, std::memory_order_relaxed); }
    void add_weak() noexcept { weak.fetch_add(1, std::memory_order_relaxed); }

    void release_strong() noexcept {
        if (strong.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            destroy_object();
            release_weak();
        }
    }
    void release_weak() noexcept {
        if (weak.fetch_sub(1, std::memory_order_acq_rel) == 1) destroy_self();
    }
    bool try_add_strong() noexcept {
        long n = strong.load(std::memory_order_relaxed);
        while (n != 0) {
            if (strong.compare_exchange_weak(n, n + 1, std::memory_order_acq_rel,
                                             std::memory_order_relaxed))
                return true;
        }
        return false;
    }

protected:
    ~ControlBlockBase() = default;
};

template <typename T>
struct PointerControlBlock final : ControlBlockBase {
    T* ptr;
    explicit PointerControlBlock(T* p) : ptr(p) {}
    void destroy_object() noexcept override { delete ptr; }
    void destroy_self() noexcept override { delete this; }
};

template <typename T>
struct InlineControlBlock final : ControlBlockBase {
    alignas(T) unsigned char storage[sizeof(T)];
    template <typename... Args>
    explicit InlineControlBlock(Args&&... args) {
        ::new (static_cast<void*>(storage)) T(std::forward<Args>(args)...);
    }
    T* object() noexcept { return std::launder(reinterpret_cast<T*>(storage)); }
    void destroy_object() noexcept override { std::destroy_at(object()); }
    void destroy_self() noexcept override { delete this; }
};

}  // namespace detail

template <typename T>
class WeakPtr;

template <typename T>
class SharedPtr {
public:
    SharedPtr() noexcept = default;
    SharedPtr(std::nullptr_t) noexcept {}

    explicit SharedPtr(T* p) {
        if (!p) return;
        try {
            cb_ = new detail::PointerControlBlock<T>(p);
        } catch (...) {
            delete p;
            throw;
        }
        ptr_ = p;
    }

    SharedPtr(const SharedPtr& o) noexcept : ptr_(o.ptr_), cb_(o.cb_) {
        if (cb_) cb_->add_strong();
    }
    SharedPtr(SharedPtr&& o) noexcept
        : ptr_(std::exchange(o.ptr_, nullptr)), cb_(std::exchange(o.cb_, nullptr)) {}

    SharedPtr& operator=(const SharedPtr& o) noexcept {
        SharedPtr(o).swap(*this);
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& o) noexcept {
        SharedPtr(std::move(o)).swap(*this);
        return *this;
    }

    ~SharedPtr() {
        if (cb_) cb_->release_strong();
    }

    void reset() noexcept { SharedPtr().swap(*this); }
    void reset(T* p) { SharedPtr(p).swap(*this); }

    void swap(SharedPtr& o) noexcept {
        std::swap(ptr_, o.ptr_);
        std::swap(cb_, o.cb_);
    }

    T* get() const noexcept { return ptr_; }
    T& operator*() const noexcept { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }
    long use_count() const noexcept {
        return cb_ ? cb_->strong.load(std::memory_order_relaxed) : 0;
    }

private:
    template <typename U>
    friend class WeakPtr;
    template <typename U, typename... Args>
    friend SharedPtr<U> make_shared(Args&&... args);

    // Adopts one strong reference already counted in cb.
    SharedPtr(T* p, detail::ControlBlockBase* cb) noexcept : ptr_(p), cb_(cb) {}

    T* ptr_ = nullptr;
    detail::ControlBlockBase* cb_ = nullptr;
};

template <typename T>
class WeakPtr {
public:
    WeakPtr() noexcept = default;
    WeakPtr(const SharedPtr<T>& s) noexcept : ptr_(s.ptr_), cb_(s.cb_) {
        if (cb_) cb_->add_weak();
    }
    WeakPtr(const WeakPtr& o) noexcept : ptr_(o.ptr_), cb_(o.cb_) {
        if (cb_) cb_->add_weak();
    }
    WeakPtr(WeakPtr&& o) noexcept
        : ptr_(std::exchange(o.ptr_, nullptr)), cb_(std::exchange(o.cb_, nullptr)) {}

    WeakPtr& operator=(const WeakPtr& o) noexcept {
        WeakPtr(o).swap(*this);
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& o) noexcept {
        WeakPtr(std::move(o)).swap(*this);
        return *this;
    }
    WeakPtr& operator=(const SharedPtr<T>& s) noexcept {
        WeakPtr(s).swap(*this);
        return *this;
    }

    ~WeakPtr() {
        if (cb_) cb_->release_weak();
    }

    void reset() noexcept { WeakPtr().swap(*this); }
    void swap(WeakPtr& o) noexcept {
        std::swap(ptr_, o.ptr_);
        std::swap(cb_, o.cb_);
    }

    long use_count() const noexcept {
        return cb_ ? cb_->strong.load(std::memory_order_relaxed) : 0;
    }
    bool expired() const noexcept { return use_count() == 0; }

    SharedPtr<T> lock() const noexcept {
        if (cb_ && cb_->try_add_strong()) return SharedPtr<T>(ptr_, cb_);
        return SharedPtr<T>();
    }

private:
    T* ptr_ = nullptr;
    detail::ControlBlockBase* cb_ = nullptr;
};

template <typename T, typename... Args>
SharedPtr<T> make_shared(Args&&... args) {
    auto* cb = new detail::InlineControlBlock<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(cb->object(), cb);
}

}  // namespace hft
