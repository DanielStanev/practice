// Reference solution for problem 09.
#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace idioms {

template <class F>
class FunctionOutput {
public:
    using iterator_category = std::output_iterator_tag;  // for legacy algorithms
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    explicit FunctionOutput(F f) : f_(std::move(f)) {}
    FunctionOutput& operator*() { return *this; }
    FunctionOutput& operator++() { return *this; }
    FunctionOutput operator++(int) { return *this; }
    template <class T>
    FunctionOutput& operator=(T&& v) {
        f_(std::forward<T>(v));
        return *this;
    }

private:
    F f_;
};

template <class F>
FunctionOutput<F> function_output(F f) {
    return FunctionOutput<F>(std::move(f));
}

class CountingOutput {
public:
    using iterator_category = std::output_iterator_tag;  // for legacy algorithms
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    CountingOutput& operator*() { return *this; }
    CountingOutput& operator++() { return *this; }
    CountingOutput operator++(int) { return *this; }
    template <class T>
    CountingOutput& operator=(const T&) {
        ++n_;
        return *this;
    }
    std::size_t count() const { return n_; }

private:
    std::size_t n_ = 0;
};

inline std::size_t intersection_size(const std::vector<int>& a, const std::vector<int>& b) {
    return std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), CountingOutput{}).count();
}

struct Fill {
    int id;
    int qty;
};

inline void split_fills(const std::vector<Fill>& fills, std::vector<Fill>& big, std::vector<Fill>& small,
                        int threshold) {
    std::partition_copy(fills.begin(), fills.end(), std::back_inserter(big), std::back_inserter(small),
                        [threshold](const Fill& f) { return f.qty >= threshold; });
}

inline long sum_of_squares(const std::vector<int>& v) {
    long total = 0;
    std::transform(v.begin(), v.end(), function_output([&total](long x) { total += x; }),
                   [](int x) { return long{x} * x; });
    return total;
}

}  // namespace idioms
