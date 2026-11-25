// Problem 04: writing a forward iterator and its const twin.
// Read README.md first. The stubs are missing member types and operators on
// purpose: add them, then implement every TODO.
#pragma once

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace idioms {

class StepRange {
public:
    class iterator {
    public:
        // TODO: member types (value_type, difference_type, iterator_concept, iterator_category).

        long operator*() const { throw std::logic_error("TODO: StepRange::iterator::operator*"); }
        iterator& operator++() { throw std::logic_error("TODO: StepRange::iterator::operator++"); }
        // TODO: postfix operator++(int)
        bool operator==(const iterator&) const {
            throw std::logic_error("TODO: StepRange::iterator::operator==");
        }

    private:
        // TODO: state
    };

    StepRange(long first, long last, long step) {
        (void)first;
        (void)last;
        (void)step;
    }
    iterator begin() const { throw std::logic_error("TODO: StepRange::begin"); }
    iterator end() const { throw std::logic_error("TODO: StepRange::end"); }
};

template <class T>
class SList {
    struct Node {
        T value;
        Node* next;
    };

public:
    template <bool Const>
    class Iter {
    public:
        // TODO: member types. `reference` is const T& when Const, T& otherwise.
        // TODO: converting constructor Iter<false> -> Iter<true> (only that direction).
        // TODO: postfix operator++(int)

        auto& operator*() const {
            throw std::logic_error("TODO: Iter::operator*");
            return node_->value;
        }
        Iter& operator++() { throw std::logic_error("TODO: Iter::operator++"); }
        bool operator==(const Iter&) const { throw std::logic_error("TODO: Iter::operator=="); }

    private:
        friend class SList;
        std::conditional_t<Const, const Node, Node>* node_ = nullptr;
    };
    using iterator = Iter<false>;
    using const_iterator = Iter<true>;

    SList() = default;
    // TODO: destructor, and forbid copying.

    void push_front(T value) {
        (void)value;
        throw std::logic_error("TODO: push_front");
    }
    std::size_t size() const { throw std::logic_error("TODO: size"); }

    iterator begin() { throw std::logic_error("TODO: begin"); }
    iterator end() { throw std::logic_error("TODO: end"); }
    const_iterator begin() const { throw std::logic_error("TODO: begin const"); }
    const_iterator end() const { throw std::logic_error("TODO: end const"); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    // Inserts after pos (which must not be end()); returns the new element.
    iterator insert_after(const_iterator pos, T value) {
        (void)pos;
        (void)value;
        throw std::logic_error("TODO: insert_after");
    }

private:
    Node* head_ = nullptr;
};

}  // namespace idioms
