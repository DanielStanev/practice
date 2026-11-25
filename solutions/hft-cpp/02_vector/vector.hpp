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

    Vector() noexcept = default;

    explicit Vector(size_type n) : Vector() {
        reserve(n);
        std::uninitialized_value_construct_n(data_, n);
        size_ = n;
    }

    Vector(size_type n, const T& value) : Vector() {
        reserve(n);
        std::uninitialized_fill_n(data_, n, value);
        size_ = n;
    }

    Vector(std::initializer_list<T> init) : Vector() {
        reserve(init.size());
        std::uninitialized_copy(init.begin(), init.end(), data_);
        size_ = init.size();
    }

    Vector(const Vector& other) : Vector() {
        reserve(other.size_);
        std::uninitialized_copy(other.begin(), other.end(), data_);
        size_ = other.size_;
    }

    Vector(Vector&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)),
          size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)) {}

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector tmp(other);
            swap(tmp);
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            Vector tmp(std::move(other));
            swap(tmp);
        }
        return *this;
    }

    ~Vector() {
        std::destroy_n(data_, size_);
        deallocate(data_, capacity_);
    }

    size_type size() const noexcept { return size_; }
    size_type capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }
    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    T& operator[](size_type i) noexcept { return data_[i]; }
    const T& operator[](size_type i) const noexcept { return data_[i]; }

    T& at(size_type i) {
        if (i >= size_) throw std::out_of_range("Vector::at");
        return data_[i];
    }
    const T& at(size_type i) const {
        if (i >= size_) throw std::out_of_range("Vector::at");
        return data_[i];
    }

    T& front() noexcept { return data_[0]; }
    T& back() noexcept { return data_[size_ - 1]; }
    const T& front() const noexcept { return data_[0]; }
    const T& back() const noexcept { return data_[size_ - 1]; }

    iterator begin() noexcept { return data_; }
    iterator end() noexcept { return data_ + size_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator end() const noexcept { return data_ + size_; }

    void reserve(size_type n) {
        if (n <= capacity_) return;
        reallocate(n);
    }

    void push_back(const T& v) { emplace_back(v); }
    void push_back(T&& v) { emplace_back(std::move(v)); }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (size_ < capacity_) {
            std::construct_at(data_ + size_, std::forward<Args>(args)...);
            return data_[size_++];
        }
        // Construct the new element in the new buffer first: args may alias an
        // element of the old buffer.
        size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
        T* fresh = allocate(new_cap);
        try {
            std::construct_at(fresh + size_, std::forward<Args>(args)...);
        } catch (...) {
            deallocate(fresh, new_cap);
            throw;
        }
        try {
            relocate(data_, size_, fresh);
        } catch (...) {
            std::destroy_at(fresh + size_);
            deallocate(fresh, new_cap);
            throw;
        }
        std::destroy_n(data_, size_);
        deallocate(data_, capacity_);
        data_ = fresh;
        capacity_ = new_cap;
        return data_[size_++];
    }

    void pop_back() noexcept {
        --size_;
        std::destroy_at(data_ + size_);
    }

    void resize(size_type n) {
        if (n < size_) {
            std::destroy(data_ + n, data_ + size_);
        } else if (n > size_) {
            reserve(std::max(n, capacity_ * 2));
            std::uninitialized_value_construct(data_ + size_, data_ + n);
        }
        size_ = n;
    }

    void resize(size_type n, const T& value) {
        if (n < size_) {
            std::destroy(data_ + n, data_ + size_);
        } else if (n > size_) {
            if (n > capacity_) {
                T copy(value);  // value may alias an element
                reserve(std::max(n, capacity_ * 2));
                std::uninitialized_fill(data_ + size_, data_ + n, copy);
            } else {
                std::uninitialized_fill(data_ + size_, data_ + n, value);
            }
        }
        size_ = n;
    }

    void clear() noexcept {
        std::destroy_n(data_, size_);
        size_ = 0;
    }

    void shrink_to_fit() {
        if (capacity_ == size_) return;
        if (size_ == 0) {
            deallocate(data_, capacity_);
            data_ = nullptr;
            capacity_ = 0;
            return;
        }
        reallocate(size_);
    }

    iterator erase(const_iterator pos) {
        T* p = data_ + (pos - data_);
        std::move(p + 1, end(), p);
        pop_back();
        return p;
    }

    void swap(Vector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    friend bool operator==(const Vector& a, const Vector& b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end());
    }

private:
    static T* allocate(size_type n) {
        return static_cast<T*>(::operator new(n * sizeof(T), std::align_val_t{alignof(T)}));
    }
    static void deallocate(T* p, size_type n) noexcept {
        (void)n;  // the unsized form is portable (Clang needs -fsized-deallocation for the sized one)
        if (p) ::operator delete(p, std::align_val_t{alignof(T)});
    }

    // Move elements if that cannot throw (or if T cannot be copied),
    // otherwise copy so a failure leaves the source untouched.
    static void relocate(T* src, size_type n, T* dst) {
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(src, n, dst);
        } else {
            std::uninitialized_copy_n(src, n, dst);
        }
    }

    void reallocate(size_type new_cap) {
        T* fresh = allocate(new_cap);
        try {
            relocate(data_, size_, fresh);
        } catch (...) {
            deallocate(fresh, new_cap);
            throw;
        }
        std::destroy_n(data_, size_);
        deallocate(data_, capacity_);
        data_ = fresh;
        capacity_ = new_cap;
    }

    T* data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;
};

}  // namespace hft
