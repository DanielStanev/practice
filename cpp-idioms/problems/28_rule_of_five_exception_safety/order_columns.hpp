// Problem 28: rule of five with two resources.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace idioms {

class OrderColumns {
public:
    OrderColumns() = default;
    explicit OrderColumns(std::size_t n) {
        (void)n;
        throw std::logic_error("TODO: OrderColumns(n)");
    }
    // TODO: copy constructor, copy assignment, move constructor, move assignment, destructor.

    std::size_t size() const { return n_; }
    double& price(std::size_t i) { return prices_[i]; }
    long& qty(std::size_t i) { return sizes_[i]; }
    const double& price(std::size_t i) const { return prices_[i]; }
    const long& qty(std::size_t i) const { return sizes_[i]; }
    const double* prices_data() const { return prices_; }

    void swap(OrderColumns& other) noexcept {
        (void)other;
        // TODO
    }

private:
    double* prices_ = nullptr;
    long* sizes_ = nullptr;
    std::size_t n_ = 0;
};

// TODO: free swap

}  // namespace idioms
