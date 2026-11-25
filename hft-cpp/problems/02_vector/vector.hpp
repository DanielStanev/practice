// Problem 02: Vector with the strong exception guarantee
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
// Do not use std::vector (or any other std container) inside.
#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hft {

template <typename T>
class Vector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;

    Vector() = default;

    // n value-initialised elements.
    explicit Vector(size_type n) {
        (void)n;
        throw std::logic_error("TODO: Vector(n)");
    }

    Vector(size_type n, const T& value) {
        (void)n;
        (void)value;
        throw std::logic_error("TODO: Vector(n, value)");
    }

    Vector(std::initializer_list<T> init) {
        (void)init;
        throw std::logic_error("TODO: Vector(initializer_list)");
    }

    Vector(const Vector& other) {
        (void)other;
        throw std::logic_error("TODO: copy constructor");
    }

    Vector(Vector&& other) {
        (void)other;
        throw std::logic_error("TODO: move constructor");
    }

    Vector& operator=(const Vector& other) {
        (void)other;
        throw std::logic_error("TODO: copy assignment");
    }

    Vector& operator=(Vector&& other) {
        (void)other;
        throw std::logic_error("TODO: move assignment");
    }

    ~Vector() {
        // TODO: destroy elements and free the buffer.
    }

    size_type size() const { return size_; }
    size_type capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    T* data() { return data_; }
    const T* data() const { return data_; }

    // Unchecked access.
    T& operator[](size_type i) { return data_[i]; }
    const T& operator[](size_type i) const { return data_[i]; }

    // Checked access: throws std::out_of_range.
    T& at(size_type i) {
        (void)i;
        throw std::logic_error("TODO: at");
    }
    const T& at(size_type i) const {
        (void)i;
        throw std::logic_error("TODO: at const");
    }

    T& front() { return data_[0]; }
    T& back() { return data_[size_ - 1]; }
    const T& front() const { return data_[0]; }
    const T& back() const { return data_[size_ - 1]; }

    iterator begin() { return data_; }
    iterator end() { return data_ + size_; }
    const_iterator begin() const { return data_; }
    const_iterator end() const { return data_ + size_; }

    // Grow capacity to at least n without constructing anything.
    // Strong exception guarantee.
    void reserve(size_type n) {
        (void)n;
        throw std::logic_error("TODO: reserve");
    }

    void push_back(const T& v) { emplace_back(v); }
    void push_back(T&& v) { emplace_back(std::move(v)); }

    // Constructs in place and returns a reference to the new element.
    // Strong exception guarantee. args may refer to an existing element.
    template <typename... Args>
    T& emplace_back(Args&&... args) {
        ((void)args, ...);
        throw std::logic_error("TODO: emplace_back");
    }

    void pop_back() {
        throw std::logic_error("TODO: pop_back");
    }

    void resize(size_type n) {
        (void)n;
        throw std::logic_error("TODO: resize(n)");
    }

    void resize(size_type n, const T& value) {
        (void)n;
        (void)value;
        throw std::logic_error("TODO: resize(n, value)");
    }

    // Destroys all elements, keeps capacity.
    void clear() {
        throw std::logic_error("TODO: clear");
    }

    // Reduces capacity to size().
    void shrink_to_fit() {
        throw std::logic_error("TODO: shrink_to_fit");
    }

    // Removes the element at pos, shifting the tail down. Returns an iterator
    // to the element that followed the removed one.
    iterator erase(const_iterator pos) {
        (void)pos;
        throw std::logic_error("TODO: erase");
    }

    void swap(Vector& other) {
        (void)other;
        throw std::logic_error("TODO: swap");
    }

    friend bool operator==(const Vector& a, const Vector& b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end());
    }

private:
    T* data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;
};

}  // namespace hft
