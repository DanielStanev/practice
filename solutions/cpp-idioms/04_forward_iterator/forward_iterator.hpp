// Reference solution for problem 04.
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
        using value_type = long;
        using difference_type = std::ptrdiff_t;
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::input_iterator_tag;

        iterator() = default;
        long operator*() const { return value_; }
        iterator& operator++() {
            value_ += step_;
            return *this;
        }
        iterator operator++(int) {
            iterator t = *this;
            ++*this;
            return t;
        }
        bool operator==(const iterator& o) const { return value_ == o.value_; }

    private:
        friend class StepRange;
        iterator(long v, long s) : value_(v), step_(s) {}
        long value_ = 0;
        long step_ = 1;
    };

    StepRange(long first, long last, long step) : first_(first), step_(step) {
        if (step <= 0) throw std::invalid_argument("StepRange: step must be positive");
        long n = last > first ? (last - first + step - 1) / step : 0;
        end_ = first + n * step;
    }
    iterator begin() const { return {first_, step_}; }
    iterator end() const { return {end_, step_}; }

private:
    long first_, end_, step_;
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
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<Const, const T&, T&>;
        using pointer = std::conditional_t<Const, const T*, T*>;
        using iterator_category = std::forward_iterator_tag;

        Iter() = default;
        // A template, so that in Iter<false> it is not mistaken for the copy constructor.
        template <bool C = Const>
            requires C
        Iter(const Iter<false>& o) : node_(o.node_) {}

        reference operator*() const { return node_->value; }
        pointer operator->() const { return &node_->value; }
        Iter& operator++() {
            node_ = node_->next;
            return *this;
        }
        Iter operator++(int) {
            Iter t = *this;
            ++*this;
            return t;
        }
        bool operator==(const Iter& o) const { return node_ == o.node_; }

    private:
        friend class SList;
        friend class Iter<!Const>;
        using NodePtr = std::conditional_t<Const, const Node, Node>*;
        explicit Iter(NodePtr n) : node_(n) {}
        NodePtr node_ = nullptr;
    };
    using iterator = Iter<false>;
    using const_iterator = Iter<true>;

    SList() = default;
    SList(const SList&) = delete;
    SList& operator=(const SList&) = delete;
    ~SList() {
        while (head_) {
            Node* n = head_->next;
            delete head_;
            head_ = n;
        }
    }

    void push_front(T value) { head_ = new Node{std::move(value), head_}; }
    std::size_t size() const {
        std::size_t n = 0;
        for (Node* p = head_; p; p = p->next) ++n;
        return n;
    }

    iterator begin() { return iterator(head_); }
    iterator end() { return iterator(nullptr); }
    const_iterator begin() const { return const_iterator(head_); }
    const_iterator end() const { return const_iterator(nullptr); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    iterator insert_after(const_iterator pos, T value) {
        Node* n = const_cast<Node*>(pos.node_);  // we own the node; const only protects the value
        n->next = new Node{std::move(value), n->next};
        return iterator(n->next);
    }

private:
    Node* head_ = nullptr;
};

}  // namespace idioms
