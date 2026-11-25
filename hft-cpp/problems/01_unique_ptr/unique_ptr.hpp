// Problem 01: UniquePtr
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
#pragma once

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hft {

// Default deleter: calls delete. Must be an empty type so that UniquePtr<T>
// can be exactly the size of a raw pointer.
template <typename T>
struct DefaultDelete {
    constexpr DefaultDelete() noexcept = default;

    // TODO: allow DefaultDelete<Derived> to convert to DefaultDelete<Base>
    // (needed for UniquePtr<Derived> -> UniquePtr<Base>).

    void operator()(T* p) const {
        // TODO
        (void)p;
        throw std::logic_error("TODO: DefaultDelete::operator()");
    }
};

template <typename T, typename Deleter = DefaultDelete<T>>
class UniquePtr {
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    UniquePtr() = default;
    UniquePtr(std::nullptr_t) {}

    explicit UniquePtr(T* p) {
        (void)p;
        throw std::logic_error("TODO: UniquePtr(T*)");
    }

    UniquePtr(T* p, Deleter d) {
        (void)p;
        (void)d;
        throw std::logic_error("TODO: UniquePtr(T*, Deleter)");
    }

    // TODO: UniquePtr must not be copyable.

    UniquePtr(UniquePtr&& other) {
        (void)other;
        throw std::logic_error("TODO: move constructor");
    }

    // Converting move: UniquePtr<Derived> -> UniquePtr<Base>.
    // TODO: constrain this so it only exists when U* converts to T* and E
    // converts to Deleter.
    template <typename U, typename E>
    UniquePtr(UniquePtr<U, E>&& other) {
        (void)other;
        throw std::logic_error("TODO: converting move constructor");
    }

    UniquePtr& operator=(UniquePtr&& other) {
        (void)other;
        throw std::logic_error("TODO: move assignment");
    }

    // TODO: constrain like the converting constructor.
    template <typename U, typename E>
    UniquePtr& operator=(UniquePtr<U, E>&& other) {
        (void)other;
        throw std::logic_error("TODO: converting move assignment");
    }

    UniquePtr& operator=(std::nullptr_t) {
        throw std::logic_error("TODO: operator=(nullptr)");
    }

    ~UniquePtr() {
        // TODO: destroy the owned object (if any) with the deleter.
    }

    T* get() const { return nullptr; /* TODO */ }
    Deleter& get_deleter() { return deleter_; }
    const Deleter& get_deleter() const { return deleter_; }

    // Gives up ownership without destroying; returns the old pointer.
    T* release() { throw std::logic_error("TODO: release"); }

    // Takes ownership of p and destroys the previously owned object.
    void reset(T* p = nullptr) {
        (void)p;
        throw std::logic_error("TODO: reset");
    }

    void swap(UniquePtr& other) {
        (void)other;
        throw std::logic_error("TODO: swap");
    }

    T& operator*() const { throw std::logic_error("TODO: operator*"); }
    T* operator->() const { throw std::logic_error("TODO: operator->"); }
    explicit operator bool() const { return false; /* TODO */ }

    friend bool operator==(const UniquePtr& p, std::nullptr_t) { return !p; }

private:
    // TODO: an empty Deleter must not make UniquePtr bigger than T*.
    T* ptr_ = nullptr;
    Deleter deleter_{};
};

template <typename T, typename... Args>
UniquePtr<T> make_unique(Args&&... args) {
    // TODO
    ((void)args, ...);
    throw std::logic_error("TODO: make_unique");
}

}  // namespace hft
