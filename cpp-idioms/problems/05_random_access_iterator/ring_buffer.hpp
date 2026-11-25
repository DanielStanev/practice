// Problem 05: a random access iterator for a ring buffer.
// Read README.md first. The iterator stub is incomplete on purpose: add the
// missing member types and operators until std::random_access_iterator holds.
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
    public:
        // TODO: member types, converting constructor, and the full set of
        // random access operations (see README).
        auto& operator*() const {
            throw std::logic_error("TODO: Iter::operator*");
            return buf_->storage_[0];
        }
        Iter& operator++() { throw std::logic_error("TODO: Iter::operator++"); }
        bool operator==(const Iter&) const { throw std::logic_error("TODO: Iter::operator=="); }

    private:
        friend class RingBuffer;
        std::conditional_t<Const, const RingBuffer, RingBuffer>* buf_ = nullptr;
        std::ptrdiff_t index_ = 0;
    };
    using iterator = Iter<false>;
    using const_iterator = Iter<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    bool push_back(T value) {
        (void)value;
        throw std::logic_error("TODO: push_back");
    }
    void pop_front() { throw std::logic_error("TODO: pop_front"); }

    std::size_t size() const { throw std::logic_error("TODO: size"); }
    bool empty() const { return size() == 0; }
    bool full() const { return size() == N; }
    static constexpr std::size_t capacity() { return N; }

    T& operator[](std::size_t i) {
        (void)i;
        throw std::logic_error("TODO: operator[]");
    }
    const T& operator[](std::size_t i) const {
        (void)i;
        throw std::logic_error("TODO: operator[] const");
    }
    T& front() { return (*this)[0]; }
    T& back() { return (*this)[size() - 1]; }

    iterator begin() { throw std::logic_error("TODO: begin"); }
    iterator end() { throw std::logic_error("TODO: end"); }
    const_iterator begin() const { throw std::logic_error("TODO: begin const"); }
    const_iterator end() const { throw std::logic_error("TODO: end const"); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }
    // TODO: rbegin/rend (const and non-const)

private:
    std::array<T, N> storage_{};
    // TODO: head index and size
};

}  // namespace idioms
