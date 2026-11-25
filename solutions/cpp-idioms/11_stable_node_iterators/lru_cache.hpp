// Reference solution for problem 11.
#pragma once

#include <cstddef>
#include <iterator>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace idioms {

template <class K, class V>
class LruCache {
    struct Entry {
        K key;
        V value;
    };
    using List = std::list<Entry>;

public:
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {
        if (capacity == 0) throw std::invalid_argument("LruCache: capacity must be >= 1");
        map_.reserve(capacity);
    }

    V* get(const K& key) {
        auto found = map_.find(key);
        if (found == map_.end()) return nullptr;
        list_.splice(list_.begin(), list_, found->second);
        return &found->second->value;
    }

    void put(const K& key, V value) {
        if (auto found = map_.find(key); found != map_.end()) {
            found->second->value = std::move(value);
            list_.splice(list_.begin(), list_, found->second);
            return;
        }
        if (list_.size() < capacity_) {
            list_.push_front(Entry{key, std::move(value)});
            map_.emplace(key, list_.begin());
            return;
        }
        // Full: recycle the LRU list node and its map node.
        auto last = std::prev(list_.end());
        auto nh = map_.extract(last->key);
        last->key = key;
        last->value = std::move(value);
        list_.splice(list_.begin(), list_, last);
        nh.key() = key;
        nh.mapped() = list_.begin();
        map_.insert(std::move(nh));
        ++evictions_;
    }

    std::size_t size() const { return list_.size(); }
    std::size_t capacity() const { return capacity_; }
    std::size_t evictions() const { return evictions_; }

    std::vector<K> keys_mru_order() const {
        std::vector<K> out;
        out.reserve(list_.size());
        for (const Entry& e : list_) out.push_back(e.key);
        return out;
    }

private:
    std::size_t capacity_;
    std::size_t evictions_ = 0;
    List list_;
    std::unordered_map<K, typename List::iterator> map_;
};

}  // namespace idioms
