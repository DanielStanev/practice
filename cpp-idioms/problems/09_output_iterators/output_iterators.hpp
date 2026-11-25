// Problem 09: output iterators.
// Read README.md first. Implement everything marked TODO.
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
    explicit FunctionOutput(F f) : f_(std::move(f)) {}
    // TODO: difference_type, operator*, ++ (both), templated operator=

private:
    F f_;
};

template <class F>
FunctionOutput<F> function_output(F f) {
    return FunctionOutput<F>(std::move(f));
}

class CountingOutput {
public:
    // TODO: make this an output iterator for any value type.
    std::size_t count() const { throw std::logic_error("TODO: CountingOutput::count"); }
};

inline std::size_t intersection_size(const std::vector<int>& a, const std::vector<int>& b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: intersection_size");
}

struct Fill {
    int id;
    int qty;
};

inline void split_fills(const std::vector<Fill>& fills, std::vector<Fill>& big, std::vector<Fill>& small,
                        int threshold) {
    (void)fills;
    (void)big;
    (void)small;
    (void)threshold;
    throw std::logic_error("TODO: split_fills");
}

inline long sum_of_squares(const std::vector<int>& v) {
    (void)v;
    throw std::logic_error("TODO: sum_of_squares");
}

}  // namespace idioms
