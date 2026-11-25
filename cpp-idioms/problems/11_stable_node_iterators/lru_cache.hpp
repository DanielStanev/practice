// Problem 11: stable node iterators, splice and node handles.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstddef>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace idioms {

template <class K, class V>
class LruCache {
public:
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {}

    V* get(const K& key) {
        (void)key;
        throw std::logic_error("TODO: get");
    }

    void put(const K& key, V value) {
        (void)key;
        (void)value;
        throw std::logic_error("TODO: put");
    }

    std::size_t size() const { throw std::logic_error("TODO: size"); }
    std::size_t capacity() const { return capacity_; }
    std::size_t evictions() const { throw std::logic_error("TODO: evictions"); }

    std::vector<K> keys_mru_order() const { throw std::logic_error("TODO: keys_mru_order"); }

private:
    struct Entry {
        K key;
        V value;
    };
    std::size_t capacity_;
    // TODO: list of entries + map from key to list iterator
};

}  // namespace idioms
