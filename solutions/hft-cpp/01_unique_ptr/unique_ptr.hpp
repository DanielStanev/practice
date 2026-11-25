#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace hft {

template <typename T>
struct DefaultDelete {
    constexpr DefaultDelete() noexcept = default;
    template <typename U>
        requires std::is_convertible_v<U*, T*>
    DefaultDelete(const DefaultDelete<U>&) noexcept {}
    void operator()(T* p) const noexcept {
        static_assert(sizeof(T) > 0, "cannot delete an incomplete type");
        delete p;
    }
};

template <typename T, typename Deleter = DefaultDelete<T>>
class UniquePtr {
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    constexpr UniquePtr() noexcept = default;
    constexpr UniquePtr(std::nullptr_t) noexcept {}
    explicit UniquePtr(T* p) noexcept : ptr_(p) {}
    UniquePtr(T* p, Deleter d) noexcept : ptr_(p), deleter_(std::move(d)) {}

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(other.release()), deleter_(std::move(other.deleter_)) {}

    template <typename U, typename E>
        requires(std::is_convertible_v<U*, T*> && std::is_convertible_v<E, Deleter>)
    UniquePtr(UniquePtr<U, E>&& other) noexcept
        : ptr_(other.release()), deleter_(std::move(other.get_deleter())) {}

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
            deleter_ = std::move(other.deleter_);
        }
        return *this;
    }

    template <typename U, typename E>
        requires(std::is_convertible_v<U*, T*> && std::is_assignable_v<Deleter&, E &&>)
    UniquePtr& operator=(UniquePtr<U, E>&& other) noexcept {
        reset(other.release());
        deleter_ = std::move(other.get_deleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    ~UniquePtr() {
        if (ptr_) deleter_(ptr_);
    }

    T* get() const noexcept { return ptr_; }
    Deleter& get_deleter() noexcept { return deleter_; }
    const Deleter& get_deleter() const noexcept { return deleter_; }

    T* release() noexcept { return std::exchange(ptr_, nullptr); }

    void reset(T* p = nullptr) noexcept {
        T* old = std::exchange(ptr_, p);
        if (old) deleter_(old);
    }

    void swap(UniquePtr& other) noexcept {
        using std::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }

    T& operator*() const noexcept { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    friend bool operator==(const UniquePtr& p, std::nullptr_t) noexcept { return !p; }

private:
    T* ptr_ = nullptr;
    [[no_unique_address]] Deleter deleter_{};
};

template <typename T, typename... Args>
UniquePtr<T> make_unique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace hft
