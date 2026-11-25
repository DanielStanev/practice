// Reference solution for problem 28.
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
        auto p = std::make_unique<double[]>(n);
        auto s = std::make_unique<long[]>(n);
        prices_ = p.release();
        sizes_ = s.release();
        n_ = n;
    }
    OrderColumns(const OrderColumns& o) {
        auto p = std::make_unique<double[]>(o.n_);
        auto s = std::make_unique<long[]>(o.n_);
        std::copy_n(o.prices_, o.n_, p.get());
        std::copy_n(o.sizes_, o.n_, s.get());
        prices_ = p.release();
        sizes_ = s.release();
        n_ = o.n_;
    }
    OrderColumns& operator=(const OrderColumns& o) {
        if (this != &o) {
            OrderColumns tmp(o);
            swap(tmp);
        }
        return *this;
    }
    OrderColumns(OrderColumns&& o) noexcept
        : prices_(std::exchange(o.prices_, nullptr)),
          sizes_(std::exchange(o.sizes_, nullptr)),
          n_(std::exchange(o.n_, 0)) {}
    OrderColumns& operator=(OrderColumns&& o) noexcept {
        if (this != &o) {
            OrderColumns tmp(std::move(o));
            swap(tmp);
        }
        return *this;
    }
    ~OrderColumns() {
        delete[] prices_;
        delete[] sizes_;
    }

    std::size_t size() const { return n_; }
    double& price(std::size_t i) { return prices_[i]; }
    long& qty(std::size_t i) { return sizes_[i]; }
    const double& price(std::size_t i) const { return prices_[i]; }
    const long& qty(std::size_t i) const { return sizes_[i]; }
    const double* prices_data() const { return prices_; }

    void swap(OrderColumns& other) noexcept {
        std::swap(prices_, other.prices_);
        std::swap(sizes_, other.sizes_);
        std::swap(n_, other.n_);
    }

private:
    double* prices_ = nullptr;
    long* sizes_ = nullptr;
    std::size_t n_ = 0;
};

inline void swap(OrderColumns& a, OrderColumns& b) noexcept { a.swap(b); }

}  // namespace idioms
