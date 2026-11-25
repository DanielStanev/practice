// Problem 29: rule of zero and a deep-copying polymorphic pointer.
// Read README.md first. Implement everything marked TODO.
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
public:
    ClonePtr() = default;

    template <class U, class... Args>
    static ClonePtr make(Args&&... args) {
        ((void)args, ...);
        throw std::logic_error("TODO: ClonePtr::make");
    }

    // TODO: copy, move, assignments, destructor

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
        ((void)args, ...);
        throw std::logic_error("TODO: Portfolio::add");
    }
    double value() const { throw std::logic_error("TODO: Portfolio::value"); }
};

// TODO: moving this copies `orders`. Fix it without removing the destructor.
struct LegacyBook {
    std::vector<std::string> orders;
    LegacyBook() = default;
    ~LegacyBook() { /* imagine logging here */ }
};

}  // namespace idioms
