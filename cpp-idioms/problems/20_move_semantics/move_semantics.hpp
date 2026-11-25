// Problem 20: move semantics.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

namespace idioms {

class Buffer {
public:
    Buffer() = default;
    explicit Buffer(std::size_t size) {
        (void)size;
        throw std::logic_error("TODO: Buffer(size)");
    }
    // TODO: copy constructor, move constructor, copy and move assignment, destructor.

    std::size_t size() const { return size_; }
    char* data() { return data_; }
    const char* data() const { return data_; }

    void swap(Buffer& other) noexcept {
        (void)other;
        // TODO
    }

private:
    std::size_t size_ = 0;
    char* data_ = nullptr;
};

// TODO: free swap(Buffer&, Buffer&)

template <class T>
class Holder {
public:
    explicit Holder(T value) {
        (void)value;
        throw std::logic_error("TODO: Holder(T)");
    }
    void set(T value) {
        (void)value;
        throw std::logic_error("TODO: Holder::set");
    }
    const T& get() const& { throw std::logic_error("TODO: Holder::get const&"); }
    T get() && { throw std::logic_error("TODO: Holder::get &&"); }

private:
    T value_{};
};

template <class T>
std::vector<T> append(std::vector<T> base, const std::vector<T>& extra) {
    (void)extra;
    throw std::logic_error("TODO: append");
    return base;
}

}  // namespace idioms
