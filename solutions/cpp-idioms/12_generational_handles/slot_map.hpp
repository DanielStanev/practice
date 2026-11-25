// Reference solution for problem 12.
#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace idioms {

struct Handle {
    std::uint32_t index = 0;
    std::uint32_t generation = 0;
    bool operator==(const Handle&) const = default;
};

template <class T>
class SlotMap {
    static constexpr std::uint32_t kNone = std::numeric_limits<std::uint32_t>::max();
    struct Slot {
        std::uint32_t dense_or_next_free;
        std::uint32_t generation;
    };

public:
    Handle insert(T value) {
        std::uint32_t slot;
        if (free_head_ != kNone) {
            slot = free_head_;
            free_head_ = slots_[slot].dense_or_next_free;
        } else {
            slot = static_cast<std::uint32_t>(slots_.size());
            slots_.push_back(Slot{kNone, 1});
        }
        values_.push_back(std::move(value));
        owner_.push_back(slot);
        slots_[slot].dense_or_next_free = static_cast<std::uint32_t>(values_.size() - 1);
        return Handle{slot, slots_[slot].generation};
    }

    T* get(Handle h) {
        if (!valid(h)) return nullptr;
        return &values_[slots_[h.index].dense_or_next_free];
    }
    const T* get(Handle h) const {
        if (!valid(h)) return nullptr;
        return &values_[slots_[h.index].dense_or_next_free];
    }
    bool contains(Handle h) const { return valid(h); }

    bool erase(Handle h) {
        if (!valid(h)) return false;
        erase_dense(slots_[h.index].dense_or_next_free);
        return true;
    }

    template <class Pred>
    std::size_t erase_if(Pred pred) {
        std::size_t n = 0;
        for (std::size_t i = 0; i < values_.size();) {
            if (pred(values_[i])) {
                erase_dense(static_cast<std::uint32_t>(i));  // brings an unvisited value to i
                ++n;
            } else {
                ++i;
            }
        }
        return n;
    }

    std::size_t size() const { return values_.size(); }
    bool empty() const { return values_.empty(); }
    std::size_t slot_count() const { return slots_.size(); }

    auto begin() { return values_.begin(); }
    auto end() { return values_.end(); }
    auto begin() const { return values_.begin(); }
    auto end() const { return values_.end(); }

private:
    bool valid(Handle h) const { return h.index < slots_.size() && slots_[h.index].generation == h.generation && h.generation != 0; }

    void erase_dense(std::uint32_t dense) {
        const std::uint32_t slot = owner_[dense];
        const std::uint32_t last = static_cast<std::uint32_t>(values_.size() - 1);
        if (dense != last) {
            values_[dense] = std::move(values_[last]);
            owner_[dense] = owner_[last];
            slots_[owner_[dense]].dense_or_next_free = dense;
        }
        values_.pop_back();
        owner_.pop_back();
        Slot& s = slots_[slot];
        ++s.generation;
        if (s.generation == 0) s.generation = 1;  // wrapped: skip the never-valid generation
        s.dense_or_next_free = free_head_;
        free_head_ = slot;
    }

    std::vector<T> values_;
    std::vector<std::uint32_t> owner_;
    std::vector<Slot> slots_;
    std::uint32_t free_head_ = kNone;
};

}  // namespace idioms
