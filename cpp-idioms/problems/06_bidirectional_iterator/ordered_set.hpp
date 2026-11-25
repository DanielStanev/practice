// Problem 06: a bidirectional iterator over a binary search tree.
// Read README.md first. The iterator stub is incomplete on purpose.
#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace idioms {

template <class T>
class OrderedSet {
    struct Node {
        T value;
        Node* parent = nullptr;
        std::unique_ptr<Node> left, right;
    };

public:
    using value_type = T;

    class iterator {
    public:
        // TODO: member types, postfix ++, prefix and postfix --.
        const T& operator*() const { throw std::logic_error("TODO: iterator::operator*"); }
        iterator& operator++() { throw std::logic_error("TODO: iterator::operator++"); }
        bool operator==(const iterator&) const { throw std::logic_error("TODO: iterator::operator=="); }

    private:
        friend class OrderedSet;
        // TODO: state (e.g. the tree and the current node)
    };
    using const_iterator = iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;

    OrderedSet() = default;
    OrderedSet(const OrderedSet&) = delete;
    OrderedSet& operator=(const OrderedSet&) = delete;
    // TODO: an iterative destructor (sorted inserts make a very deep tree).

    std::pair<iterator, bool> insert(T value) {
        (void)value;
        throw std::logic_error("TODO: insert");
    }
    iterator find(const T& key) const {
        (void)key;
        throw std::logic_error("TODO: find");
    }
    iterator lower_bound(const T& key) const {
        (void)key;
        throw std::logic_error("TODO: lower_bound");
    }
    std::size_t size() const { throw std::logic_error("TODO: size"); }

    iterator begin() const { throw std::logic_error("TODO: begin"); }
    iterator end() const { throw std::logic_error("TODO: end"); }
    // TODO: rbegin(), rend()

private:
    std::unique_ptr<Node> root_;
};

}  // namespace idioms
