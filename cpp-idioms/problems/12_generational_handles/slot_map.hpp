// Problem 12: generational slot map.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstddef>
#include <cstdint>
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
public:
    Handle insert(T value) {
        (void)value;
        throw std::logic_error("TODO: insert");
    }
    T* get(Handle h) {
        (void)h;
        throw std::logic_error("TODO: get");
    }
    const T* get(Handle h) const {
        (void)h;
        throw std::logic_error("TODO: get const");
    }
    bool contains(Handle h) const { return get(h) != nullptr; }
    bool erase(Handle h) {
        (void)h;
        throw std::logic_error("TODO: erase");
    }
    template <class Pred>
    std::size_t erase_if(Pred pred) {
        (void)pred;
        throw std::logic_error("TODO: erase_if");
    }

    std::size_t size() const { return values_.size(); }
    bool empty() const { return values_.empty(); }
    std::size_t slot_count() const { throw std::logic_error("TODO: slot_count"); }  // for tests

    auto begin() { return values_.begin(); }
    auto end() { return values_.end(); }
    auto begin() const { return values_.begin(); }
    auto end() const { return values_.end(); }

private:
    std::vector<T> values_;
    // TODO: slots, owner (dense -> slot) and free list
};

}  // namespace idioms
