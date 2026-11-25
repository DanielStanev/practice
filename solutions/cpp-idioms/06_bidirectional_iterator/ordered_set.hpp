// Reference solution for problem 06.
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

    static Node* leftmost(Node* n) {
        while (n && n->left) n = n->left.get();
        return n;
    }
    static Node* rightmost(Node* n) {
        while (n && n->right) n = n->right.get();
        return n;
    }

public:
    using value_type = T;

    class iterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = const T&;
        using pointer = const T*;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator() = default;
        const T& operator*() const { return node_->value; }
        const T* operator->() const { return &node_->value; }

        iterator& operator++() {
            if (node_->right) {
                node_ = leftmost(node_->right.get());
            } else {
                Node* p = node_->parent;
                while (p && node_ == p->right.get()) {
                    node_ = p;
                    p = p->parent;
                }
                node_ = p;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator t = *this;
            ++*this;
            return t;
        }
        iterator& operator--() {
            if (!node_) {
                node_ = rightmost(set_->root_.get());
            } else if (node_->left) {
                node_ = rightmost(node_->left.get());
            } else {
                Node* p = node_->parent;
                while (p && node_ == p->left.get()) {
                    node_ = p;
                    p = p->parent;
                }
                node_ = p;
            }
            return *this;
        }
        iterator operator--(int) {
            iterator t = *this;
            --*this;
            return t;
        }
        bool operator==(const iterator& o) const { return node_ == o.node_; }

    private:
        friend class OrderedSet;
        iterator(const OrderedSet* s, Node* n) : set_(s), node_(n) {}
        const OrderedSet* set_ = nullptr;
        Node* node_ = nullptr;
    };
    using const_iterator = iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;

    OrderedSet() = default;
    OrderedSet(const OrderedSet&) = delete;
    OrderedSet& operator=(const OrderedSet&) = delete;
    ~OrderedSet() {
        std::vector<std::unique_ptr<Node>> stack;
        if (root_) stack.push_back(std::move(root_));
        while (!stack.empty()) {
            std::unique_ptr<Node> n = std::move(stack.back());
            stack.pop_back();
            if (n->left) stack.push_back(std::move(n->left));
            if (n->right) stack.push_back(std::move(n->right));
        }
    }

    std::pair<iterator, bool> insert(T value) {
        Node* parent = nullptr;
        std::unique_ptr<Node>* slot = &root_;
        while (*slot) {
            parent = slot->get();
            if (value < parent->value) slot = &parent->left;
            else if (parent->value < value) slot = &parent->right;
            else return {iterator(this, parent), false};
        }
        *slot = std::make_unique<Node>(Node{std::move(value), parent, nullptr, nullptr});
        ++size_;
        return {iterator(this, slot->get()), true};
    }
    iterator find(const T& key) const {
        iterator it = lower_bound(key);
        return (it != end() && !(key < *it)) ? it : end();
    }
    iterator lower_bound(const T& key) const {
        Node* n = root_.get();
        Node* best = nullptr;
        while (n) {
            if (n->value < key) {
                n = n->right.get();
            } else {
                best = n;
                n = n->left.get();
            }
        }
        return iterator(this, best);
    }
    std::size_t size() const { return size_; }

    iterator begin() const { return iterator(this, leftmost(root_.get())); }
    iterator end() const { return iterator(this, nullptr); }
    reverse_iterator rbegin() const { return reverse_iterator(end()); }
    reverse_iterator rend() const { return reverse_iterator(begin()); }

private:
    std::unique_ptr<Node> root_;
    std::size_t size_ = 0;
};

}  // namespace idioms
