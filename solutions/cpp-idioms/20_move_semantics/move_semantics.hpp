// Reference solution for problem 20.
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
    explicit Buffer(std::size_t size) : size_(size), data_(size ? new char[size]() : nullptr) {}
    Buffer(const Buffer& o) : size_(o.size_), data_(o.size_ ? new char[o.size_] : nullptr) {
        if (size_) std::memcpy(data_, o.data_, size_);
    }
    Buffer(Buffer&& o) noexcept : size_(std::exchange(o.size_, 0)), data_(std::exchange(o.data_, nullptr)) {}
    Buffer& operator=(const Buffer& o) {
        if (this != &o) {
            Buffer tmp(o);
            swap(tmp);
        }
        return *this;
    }
    Buffer& operator=(Buffer&& o) noexcept {
        if (this != &o) {
            delete[] data_;
            size_ = std::exchange(o.size_, 0);
            data_ = std::exchange(o.data_, nullptr);
        }
        return *this;
    }
    ~Buffer() { delete[] data_; }

    std::size_t size() const { return size_; }
    char* data() { return data_; }
    const char* data() const { return data_; }

    void swap(Buffer& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

private:
    std::size_t size_ = 0;
    char* data_ = nullptr;
};

inline void swap(Buffer& a, Buffer& b) noexcept { a.swap(b); }

template <class T>
class Holder {
public:
    explicit Holder(T value) : value_(std::move(value)) {}
    void set(T value) { value_ = std::move(value); }
    const T& get() const& { return value_; }
    T get() && { return std::move(value_); }

private:
    T value_{};
};

template <class T>
std::vector<T> append(std::vector<T> base, const std::vector<T>& extra) {
    base.insert(base.end(), extra.begin(), extra.end());
    return base;
}

}  // namespace idioms
