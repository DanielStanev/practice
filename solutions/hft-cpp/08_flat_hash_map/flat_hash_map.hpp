#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace hft {

template <typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
class FlatHashMap {
    using Slot = std::optional<std::pair<K, V>>;

public:
    explicit FlatHashMap(std::size_t initial_capacity = 16) {
        std::size_t cap = 8;
        while (cap < initial_capacity) cap *= 2;
        slots_.resize(cap);
    }

    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    std::size_t capacity() const noexcept { return slots_.size(); }
    double load_factor() const noexcept {
        return static_cast<double>(size_) / static_cast<double>(slots_.size());
    }

    bool insert(const K& key, V value) {
        std::size_t i = find_index(key);
        if (i != npos) return false;
        emplace_new(key, std::move(value));
        return true;
    }

    void insert_or_assign(const K& key, V value) {
        std::size_t i = find_index(key);
        if (i != npos) {
            slots_[i]->second = std::move(value);
            return;
        }
        emplace_new(key, std::move(value));
    }

    V& operator[](const K& key) {
        std::size_t i = find_index(key);
        if (i != npos) return slots_[i]->second;
        return emplace_new(key, V{});
    }

    V* find(const K& key) {
        std::size_t i = find_index(key);
        return i == npos ? nullptr : &slots_[i]->second;
    }
    const V* find(const K& key) const {
        std::size_t i = find_index(key);
        return i == npos ? nullptr : &slots_[i]->second;
    }
    bool contains(const K& key) const { return find_index(key) != npos; }

    bool erase(const K& key) {
        std::size_t i = find_index(key);
        if (i == npos) return false;
        const std::size_t mask = slots_.size() - 1;
        slots_[i].reset();
        --size_;
        // Backward-shift: pull later members of the probe run into the hole
        // when the hole lies between their home slot and their current slot.
        std::size_t hole = i;
        std::size_t j = (i + 1) & mask;
        while (slots_[j]) {
            std::size_t home = bucket(slots_[j]->first);
            // distance from home to j vs. home to hole (cyclic)
            if (((j - home) & mask) >= ((j - hole) & mask)) {
                slots_[hole] = std::move(slots_[j]);
                slots_[j].reset();
                hole = j;
            }
            j = (j + 1) & mask;
        }
        return true;
    }

    void clear() {
        for (auto& s : slots_) s.reset();
        size_ = 0;
    }

    void reserve(std::size_t n) {
        std::size_t cap = slots_.size();
        while (n * 4 > cap * 3) cap *= 2;
        if (cap != slots_.size()) rehash(cap);
    }

    template <typename F>
    void for_each(F&& f) const {
        for (const auto& s : slots_)
            if (s) f(s->first, s->second);
    }

private:
    static constexpr std::size_t npos = ~std::size_t{0};

    std::size_t bucket(const K& key) const {
        // std::hash<int> is the identity on libstdc++; mix the bits so that
        // keys like multiples of 1024 do not all land in the same bucket.
        std::uint64_t h = static_cast<std::uint64_t>(hash_(key));
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdull;
        h ^= h >> 33;
        return static_cast<std::size_t>(h) & (slots_.size() - 1);
    }

    std::size_t find_index(const K& key) const {
        const std::size_t mask = slots_.size() - 1;
        for (std::size_t i = bucket(key);; i = (i + 1) & mask) {
            if (!slots_[i]) return npos;
            if (eq_(slots_[i]->first, key)) return i;
        }
    }

    V& emplace_new(const K& key, V value) {
        if ((size_ + 1) * 4 > slots_.size() * 3) rehash(slots_.size() * 2);
        const std::size_t mask = slots_.size() - 1;
        std::size_t i = bucket(key);
        while (slots_[i]) i = (i + 1) & mask;
        slots_[i].emplace(key, std::move(value));
        ++size_;
        return slots_[i]->second;
    }

    void rehash(std::size_t new_cap) {
        std::vector<Slot> old(new_cap);
        old.swap(slots_);
        const std::size_t mask = new_cap - 1;
        for (auto& s : old) {
            if (!s) continue;
            std::size_t i = bucket(s->first);
            while (slots_[i]) i = (i + 1) & mask;
            slots_[i] = std::move(s);
        }
    }

    std::vector<Slot> slots_;
    std::size_t size_ = 0;
    [[no_unique_address]] Hash hash_{};
    [[no_unique_address]] KeyEqual eq_{};
};

}  // namespace hft
