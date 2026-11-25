// Reference solution for problem 05.
#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace idioms {

template <class T, std::size_t N>
class RingBuffer {
    static_assert(N > 0);

public:
    using value_type = T;
    using size_type = std::size_t;

    template <bool Const>
    class Iter {
        using Buf = std::conditional_t<Const, const RingBuffer, RingBuffer>;

    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<Const, const T&, T&>;
        using pointer = std::conditional_t<Const, const T*, T*>;
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;

        Iter() = default;
        template <bool C = Const>
            requires C
        Iter(const Iter<false>& o) : buf_(o.buf_), index_(o.index_) {}

        reference operator*() const { return buf_->slot(index_); }
        pointer operator->() const { return &**this; }
        reference operator[](difference_type n) const { return *(*this + n); }

        Iter& operator+=(difference_type n) {
            index_ += n;
            return *this;
        }
        Iter& operator-=(difference_type n) { return *this += -n; }
        Iter& operator++() { return *this += 1; }
        Iter& operator--() { return *this -= 1; }
        Iter operator++(int) {
            Iter t = *this;
            ++*this;
            return t;
        }
        Iter operator--(int) {
            Iter t = *this;
            --*this;
            return t;
        }
        friend Iter operator+(Iter it, difference_type n) { return it += n; }
        friend Iter operator+(difference_type n, Iter it) { return it += n; }
        friend Iter operator-(Iter it, difference_type n) { return it -= n; }
        friend difference_type operator-(const Iter& a, const Iter& b) { return a.index_ - b.index_; }
        bool operator==(const Iter& o) const { return index_ == o.index_; }
        auto operator<=>(const Iter& o) const { return index_ <=> o.index_; }

    private:
        friend class RingBuffer;
        friend class Iter<!Const>;
        Iter(Buf* b, std::ptrdiff_t i) : buf_(b), index_(i) {}
        Buf* buf_ = nullptr;
        std::ptrdiff_t index_ = 0;
    };
    using iterator = Iter<false>;
    using const_iterator = Iter<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    bool push_back(T value) {
        if (full()) return false;
        storage_[(head_ + size_) % N] = std::move(value);
        ++size_;
        return true;
    }
    void pop_front() {
        head_ = (head_ + 1) % N;
        --size_;
    }

    std::size_t size() const { return size_; }
    bool empty() const { return size() == 0; }
    bool full() const { return size() == N; }
    static constexpr std::size_t capacity() { return N; }

    T& operator[](std::size_t i) { return slot(static_cast<std::ptrdiff_t>(i)); }
    const T& operator[](std::size_t i) const { return slot(static_cast<std::ptrdiff_t>(i)); }
    T& front() { return (*this)[0]; }
    T& back() { return (*this)[size() - 1]; }

    iterator begin() { return {this, 0}; }
    iterator end() { return {this, static_cast<std::ptrdiff_t>(size_)}; }
    const_iterator begin() const { return {this, 0}; }
    const_iterator end() const { return {this, static_cast<std::ptrdiff_t>(size_)}; }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

private:
    T& slot(std::ptrdiff_t i) { return storage_[(head_ + static_cast<std::size_t>(i)) % N]; }
    const T& slot(std::ptrdiff_t i) const { return storage_[(head_ + static_cast<std::size_t>(i)) % N]; }

    std::array<T, N> storage_{};
    std::size_t head_ = 0;
    std::size_t size_ = 0;
};

}  // namespace idioms
