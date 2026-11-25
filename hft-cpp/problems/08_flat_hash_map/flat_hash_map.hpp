// Problem 08: open-addressing flat hash map
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
// Do not use std::unordered_map / std::map inside. std::vector (or raw
// storage) for the slot array is fine.
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hft {

template <typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
class FlatHashMap {
public:
    // Capacity is rounded up to a power of two.
    explicit FlatHashMap(std::size_t initial_capacity = 16) {
        (void)initial_capacity;
        // TODO
    }

    std::size_t size() const noexcept { return 0; /* TODO */ }
    bool empty() const noexcept { return size() == 0; }
    // Number of slots (a power of two).
    std::size_t capacity() const noexcept { return 0; /* TODO */ }
    double load_factor() const noexcept { return 0.0; /* TODO */ }

    // Inserts if key is absent. Returns false (and changes nothing) if present.
    bool insert(const K& key, V value) {
        (void)key;
        (void)value;
        throw std::logic_error("TODO: insert");
    }

    void insert_or_assign(const K& key, V value) {
        (void)key;
        (void)value;
        throw std::logic_error("TODO: insert_or_assign");
    }

    // Default-constructs the value if key is absent.
    V& operator[](const K& key) {
        (void)key;
        throw std::logic_error("TODO: operator[]");
    }

    // nullptr if absent. The pointer is invalidated by any insert or erase.
    V* find(const K& key) {
        (void)key;
        throw std::logic_error("TODO: find");
    }
    const V* find(const K& key) const {
        (void)key;
        throw std::logic_error("TODO: find const");
    }
    bool contains(const K& key) const { return find(key) != nullptr; }

    // Returns whether the key was present. Must not leave tombstones.
    bool erase(const K& key) {
        (void)key;
        throw std::logic_error("TODO: erase");
    }

    // Removes everything, keeps capacity.
    void clear() { throw std::logic_error("TODO: clear"); }

    // After reserve(n), inserting until size() == n does not rehash.
    void reserve(std::size_t n) {
        (void)n;
        throw std::logic_error("TODO: reserve");
    }

    // Calls f(key, value) for every element, in unspecified order.
    template <typename F>
    void for_each(F&& f) const {
        (void)f;
        throw std::logic_error("TODO: for_each");
    }

private:
    // TODO: slot storage, size, hasher, key_equal.
};

}  // namespace hft
