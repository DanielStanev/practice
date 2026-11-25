// Reference solution for problem 29.
#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace idioms {

template <class T>
class ClonePtr {
    static_assert(std::has_virtual_destructor_v<T>, "ClonePtr<T> deletes through T*: T needs a virtual destructor");

public:
    ClonePtr() = default;

    template <class U, class... Args>
    static ClonePtr make(Args&&... args) {
        static_assert(std::is_base_of_v<T, U>);
        ClonePtr p;
        p.ptr_ = new U(std::forward<Args>(args)...);
        p.copier_ = [](const T* src) -> T* { return new U(*static_cast<const U*>(src)); };
        return p;
    }

    ClonePtr(const ClonePtr& o) : ptr_(o.ptr_ ? o.copier_(o.ptr_) : nullptr), copier_(o.copier_) {}
    ClonePtr(ClonePtr&& o) noexcept
        : ptr_(std::exchange(o.ptr_, nullptr)), copier_(std::exchange(o.copier_, nullptr)) {}
    ClonePtr& operator=(ClonePtr o) noexcept {  // by value: copy-and-swap for both forms
        swap(o);
        return *this;
    }
    ~ClonePtr() { delete ptr_; }

    void swap(ClonePtr& o) noexcept {
        std::swap(ptr_, o.ptr_);
        std::swap(copier_, o.copier_);
    }

    T* get() const noexcept { return ptr_; }
    T& operator*() const { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
    T* ptr_ = nullptr;
    T* (*copier_)(const T*) = nullptr;
};

struct Instrument {
    virtual ~Instrument() = default;
    virtual double value() const = 0;
};

class Portfolio {
public:
    std::string name;
    std::vector<ClonePtr<Instrument>> legs;

    template <class U, class... Args>
    void add(Args&&... args) {
        legs.push_back(ClonePtr<Instrument>::make<U>(std::forward<Args>(args)...));
    }
    double value() const {
        double v = 0;
        for (const auto& leg : legs) v += leg->value();
        return v;
    }
};

struct LegacyBook {
    std::vector<std::string> orders;
    LegacyBook() = default;
    LegacyBook(const LegacyBook&) = default;
    LegacyBook(LegacyBook&&) = default;
    LegacyBook& operator=(const LegacyBook&) = default;
    LegacyBook& operator=(LegacyBook&&) = default;
    ~LegacyBook() { /* imagine logging here */ }
};

}  // namespace idioms
