// Problem 03: SharedPtr / WeakPtr with a control block
// Read README.md first. Implement everything marked TODO.
// You may add private members, helper classes and friends, but keep the
// public interface.
#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace hft {

namespace detail {

// TODO: design the control block. It needs a strong count, a weak count, and
// a way to (1) destroy the managed object and (2) free itself, which differ
// between SharedPtr(new T) and make_shared<T>().
struct ControlBlockBase {};

}  // namespace detail

template <typename T>
class WeakPtr;

template <typename T>
class SharedPtr {
public:
    SharedPtr() = default;
    SharedPtr(std::nullptr_t) {}

    // Takes ownership of p (allocates a separate control block).
    explicit SharedPtr(T* p) {
        (void)p;
        throw std::logic_error("TODO: SharedPtr(T*)");
    }

    SharedPtr(const SharedPtr& o) {
        (void)o;
        throw std::logic_error("TODO: copy constructor");
    }
    SharedPtr(SharedPtr&& o) {
        (void)o;
        throw std::logic_error("TODO: move constructor");
    }
    SharedPtr& operator=(const SharedPtr& o) {
        (void)o;
        throw std::logic_error("TODO: copy assignment");
    }
    SharedPtr& operator=(SharedPtr&& o) {
        (void)o;
        throw std::logic_error("TODO: move assignment");
    }

    ~SharedPtr() {
        // TODO: drop one strong reference.
    }

    void reset() { throw std::logic_error("TODO: reset()"); }
    void reset(T* p) {
        (void)p;
        throw std::logic_error("TODO: reset(T*)");
    }
    void swap(SharedPtr& o) {
        (void)o;
        throw std::logic_error("TODO: swap");
    }

    T* get() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; }

    // Number of SharedPtrs sharing ownership (0 if empty).
    long use_count() const { return 0; /* TODO */ }

private:
    template <typename U>
    friend class WeakPtr;
    template <typename U, typename... Args>
    friend SharedPtr<U> make_shared(Args&&... args);

    T* ptr_ = nullptr;
    detail::ControlBlockBase* cb_ = nullptr;
};

template <typename T>
class WeakPtr {
public:
    WeakPtr() = default;
    WeakPtr(const SharedPtr<T>& s) {
        (void)s;
        throw std::logic_error("TODO: WeakPtr(SharedPtr)");
    }
    WeakPtr(const WeakPtr& o) {
        (void)o;
        throw std::logic_error("TODO: WeakPtr copy");
    }
    WeakPtr(WeakPtr&& o) {
        (void)o;
        throw std::logic_error("TODO: WeakPtr move");
    }
    WeakPtr& operator=(const WeakPtr& o) {
        (void)o;
        throw std::logic_error("TODO: WeakPtr copy assignment");
    }
    WeakPtr& operator=(WeakPtr&& o) {
        (void)o;
        throw std::logic_error("TODO: WeakPtr move assignment");
    }
    WeakPtr& operator=(const SharedPtr<T>& s) {
        (void)s;
        throw std::logic_error("TODO: WeakPtr = SharedPtr");
    }

    ~WeakPtr() {
        // TODO: drop one weak reference.
    }

    void reset() { throw std::logic_error("TODO: WeakPtr::reset"); }
    void swap(WeakPtr& o) {
        (void)o;
        throw std::logic_error("TODO: WeakPtr::swap");
    }

    long use_count() const { return 0; /* TODO */ }
    bool expired() const { return true; /* TODO */ }

    // Returns a SharedPtr to the object if it is still alive, else empty.
    // Must be safe against a concurrent release of the last SharedPtr.
    SharedPtr<T> lock() const { throw std::logic_error("TODO: lock"); }

private:
    T* ptr_ = nullptr;
    detail::ControlBlockBase* cb_ = nullptr;
};

// Must perform exactly one heap allocation (object and control block together).
template <typename T, typename... Args>
SharedPtr<T> make_shared(Args&&... args) {
    ((void)args, ...);
    throw std::logic_error("TODO: make_shared");
}

}  // namespace hft
